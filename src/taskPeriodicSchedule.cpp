/*
 * taskPeriodicSchedule.cpp
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

#include <taskPeriodicSchedule.h>

Tasking::PeriodicScheduleTrigger::PeriodicScheduleTrigger(Tasking::Time offset) : offsetTime(offset), next(nullptr)
{
}

//---------------------------

void
Tasking::PeriodicSchedule::add(Tasking::PeriodicScheduleTrigger& newTrigger)
{
    impl.sortIn(newTrigger);
}

// ==========================

Tasking::PeriodicScheduleImpl::PeriodicScheduleImpl(void) :
    triggers(nullptr),
    activeTrigger(nullptr),
    startTimeOffPeriod_ms(0u),
    period_ms(0u)
{
}

//---------------------------

void
Tasking::PeriodicScheduleImpl::sortIn(Tasking::PeriodicScheduleTrigger& trigger)
{
    // If list of triggers is empty, trigger becomes the new head.
    if (triggers == nullptr)
    {
        triggers = &trigger;
        trigger.next = nullptr;
    }
    else
    {
        // Find the right position in the list, check if trigger is the new head
        if (trigger.offsetTime < triggers->offsetTime)
        {
            // trigger is new head
            trigger.next = triggers;
            triggers = &trigger;
        }
        else
        {
            // not head, find previous element in queue.
            PeriodicScheduleTrigger* prev = triggers;
            while ((prev->next != nullptr) && (prev->next->offsetTime < trigger.offsetTime))
            {
                prev = prev->next;
            }
            // previous element is found hang trigger after this element
            trigger.next = prev->next;
            prev->next = &trigger;
        }
    }
}

//---------------------------

void
Tasking::PeriodicScheduleImpl::pushTriggers(void)
{
    // Fire the active trigger and all following with same offset time
    activeTrigger->push();
    while ((nullptr != activeTrigger->next) && (activeTrigger->offsetTime == activeTrigger->next->offsetTime))
    {
        activeTrigger = activeTrigger->next;
        activeTrigger->push();
    }
}

//---------------------------

Tasking::Time
Tasking::PeriodicScheduleImpl::stepToNextTriggerOffset(void)
{
    // For safety assume first no trigger is available
    Tasking::Time nextAbsoluteStartTime(endOfTime);
    // Step to next trigger in list
    if (nullptr != activeTrigger)
    {
        activeTrigger = activeTrigger->next;
    }
    // Check for end of list or first run after starting the schedule
    if (nullptr == activeTrigger)
    {
        // Start the list and update start time of period
        activeTrigger = triggers;
        startTimeOffPeriod_ms += period_ms;
    }
    // Active trigger should here always valid. If triggers is a null pointer, the schedule is not started by the event
    nextAbsoluteStartTime = startTimeOffPeriod_ms + activeTrigger->offsetTime;
    return nextAbsoluteStartTime;
}
