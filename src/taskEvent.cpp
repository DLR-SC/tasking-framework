/*
 * taskEvent.cpp
 *
 * Copyright 2012-2019 German Aerospace Center (DLR) SC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <taskEvent.h>
#include <taskPeriodicSchedule.h>
#include <taskChannel.h>
#include <taskUtils.h>
#include <scheduler.h>
#include <impl/clock_impl.h>

#include "accessor.h"

Tasking::Event::Event(Scheduler& scheduler, ChannelId eventId) : Channel(eventId), impl(*this, scheduler)
{
}

//-------------------------------------

Tasking::Event::Event(Tasking::Scheduler& scheduler, const char* eventName) :
    Event(scheduler, getChannelIdFromName(eventName))
{
}

//-------------------------------------

Tasking::Event::~Event(void)
{
    impl.clock.dequeue(impl);
}

//-------------------------------------

void
Tasking::Event::setPeriodicTiming(const Tasking::Time period, const Tasking::Time offset)
{
    impl.mutex.enter();

    // If event is currently queued, change of time setting can damage clock queue. So, first dequeue
    if (impl.queued)
    {
        impl.clock.dequeue(impl);
    }

    impl.configurePeriodicTiming(period, offset);
    impl.periodicSchedule = nullptr; // Not an periodic schedule
    impl.clock.startAt(impl, impl.nextActivation_ms);

    impl.mutex.leave();
}

//-------------------------------------

void
Tasking::Event::setPeriodicSchedule(const Time period, const Time offset, PeriodicSchedule& schedule)
{
    impl.mutex.enter();
    impl.mutexLock = true; // Needed to call stop.

    if (impl.queued)
    {
        impl.clock.dequeue(impl);
    }
    // Set the schedule and use normal timing set up
    impl.setPeriodicSchedule(schedule);
    // Has periodic schedule a non empty list of triggers? If not, clean event up because it won't work.
    if (impl.periodicSchedule->triggers != nullptr)
    {
        impl.configurePeriodicTiming(period, offset);

        // Configure the period in the schedule, because it is the first adjust to previous period
        // An underflow has no effect on result
        impl.periodicSchedule->startTimeOffPeriod_ms = impl.nextActivation_ms - period;
        impl.periodicSchedule->period_ms = period;

        // Start at time of first trigger in periodic schedule if it is in the cylce
        Tasking::Time nextTriggerOffset = impl.periodicSchedule->stepToNextTriggerOffset();
        if (nextTriggerOffset < period)
        {
            impl.clock.startAt(impl, nextTriggerOffset);
        }
        else
        {
            // periodic schedule has no values inside period
            stop();
        }
    }
    else
    {
        // Periodic schedule was without triggers.
        impl.periodicSchedule = nullptr;
    }
    impl.mutexLock = false;
    impl.mutex.leave();
}

//-------------------------------------

void
Tasking::Event::setRelativeTiming(const Time delay)
{
    impl.mutex.enter();

    // If event is currently queued, change of time setting can damage clock queue. So, first dequeue
    if (impl.queued)
    {
        impl.clock.dequeue(impl);
    }

    // Relative behavior
    impl.periodical = false;
    impl.period_ms = delay;
    impl.configured = true;

    impl.mutex.leave();
}

//-------------------------------------
void
Tasking::Event::trigger(const Tasking::Time time)
{
    // Do only something if it is not configured
    if (!impl.configured)
    {
        impl.mutex.enter();

        // Remove event if it is in the clock queue.
        if (impl.queued)
        {
            impl.clock.dequeue(impl);
        }

        // If no time is given, fire the event when condition to fire are met.
        if (time == 0)
        {
            if (shallFire())
            {
                push();
            }
        }
        else // Trigger with a delay.
        {
            impl.clock.startIn(impl, time);
        }
        impl.mutex.leave();
    } // Not configured
}

//-------------------------------------

bool
Tasking::Event::isTriggered(void) const
{
    impl.mutex.enter();
    bool result = impl.queued;
    impl.mutex.leave();
    return result;
}

//-------------------------------------

void
Tasking::Event::stop(void)
{
    if (!impl.mutexLock)
    {
        impl.mutex.enter();
    }
    impl.clock.dequeue(impl);
    impl.configured = false;
    impl.periodicSchedule = nullptr;
    if (!impl.mutexLock)
    {
        impl.mutex.leave();
    }
}

//-------------------------------------

void
Tasking::Event::reset(void)
{
    impl.mutex.enter();
    // Reset operation has only an effect when the event is configured as non-periodical.
    if (impl.configured && !impl.periodical)
    {
        // Queued non-periodical events has to re-trigger by this reset.
        if (impl.queued)
        {
            impl.clock.dequeue(impl);
        }
        impl.clock.startIn(impl, impl.period_ms);
    }
    impl.mutex.leave();
    Channel::reset();
}

//-------------------------------------

bool
Tasking::Event::shallFire(void)
{
    // So long it is not overloaded the normal behavior is fire the event when it is triggered and not configured with
    // a periodic schedule
    return (nullptr == impl.periodicSchedule);
}

//-------------------------------------

void
Tasking::Event::onFire(void)
{
    // Nothing to do
}

//-------------------------------------

Tasking::Time
Tasking::Event::now(void) const
{
    return impl.clock.getTime();
}

// ====================================

Tasking::EventImpl::EventImpl(Event& event, Scheduler& scheduler) :
    parent(event),
    configured(false),
    periodical(false),
    queued(false),
    period_ms(0),
    nextActivation_ms(0),
    periodicSchedule(nullptr),
    clock(TaskingAccessor().getImpl(scheduler).clock),
    next(nullptr),
    previous(nullptr),
    mutexLock(false)
{
}

//-------------------------------------

void
Tasking::EventImpl::handle(void)
{
    // If the event is periodic the next wake up time should hand over to the clock
    mutex.enter();
    if (periodical)
    {
        if (nullptr == periodicSchedule)
        {
            // No periodic schedule to play, jump to next period
            clock.startAt(*this, (nextActivation_ms + period_ms)); // If trigger is called now clock are out of order.
        }
        else
        {
            // Play periodic schedule
            periodicSchedule->pushTriggers();
            clock.startAt(*this, periodicSchedule->stepToNextTriggerOffset());
        }
    }
    mutex.leave();

    if (parent.shallFire())
    {
        parent.onFire();

        TaskingAccessor().push(parent);
    }
}

//-------------------------------------

void
Tasking::EventImpl::setPeriodicSchedule(PeriodicSchedule& schedule)
{
    periodicSchedule = &schedule.impl;
}

//-------------------------------------

void
Tasking::EventImpl::configurePeriodicTiming(const Tasking::Time period, const Tasking::Time offset)
{
    period_ms = period;
    nextActivation_ms = offset;
    periodical = (period > 0u); // Don't activated periodical mode for the event when no period is given

    // Check if time is over, in this case the next valid start point must computed which match to offset + x * period
    if (clock.getTime() > offset)
    {
        if (period > 0u)
        {
            nextActivation_ms += ((clock.getTime() - offset) / period + 1) * period;
        }
        else
        {
            nextActivation_ms = clock.getTime();
        }
    }
    configured =
            periodical; // Configured should only set in periodical mode, else we trigger with a relative timing of 0ms
}
