/*
 * taskEvent_impl.h
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

#ifndef INCLUDE_IMPL_TASKEVENT_IMPL_H_
#define INCLUDE_IMPL_TASKEVENT_IMPL_H_

#include <mutex.h>
#include "../taskChannel.h"
#include "../taskTypes.h"

namespace Tasking {

// Forward declaration for friend definition
class Clock;
class Scheduler;
class Event;
class PeriodicSchedule;
class PeriodicScheduleImpl;

/**
 * Implementation part of an event
 *
 * @see Tasking::Event
 */
struct EventImpl
{
    /**
     * @param event Reference to the parent event instance
     * @param scheduler Reference to the scheduler responsible to execute the event.
     */
    EventImpl(Event& event, Scheduler& scheduler);

    /// Reference to event which is parent of this implementation
    Event& parent;

    /// Flag to mark a time configuration. True if periodic or relative timing is configured
    bool configured;

    /// Flag to mark the event as periodic event
    bool periodical;

    /**
     * Flag to indicated that the event is still queued by the clock. In this case the event has first removed from
     * the clock queue.
     */
    bool queued;

    /// The period in case of a periodical timer or the relative time
    Time period_ms;

    /// Next activation time.
    Time nextActivation_ms;

    /// Pointer to an schedule of periodic triggers to play
    PeriodicScheduleImpl* periodicSchedule;

    /// Reference to the connected clock which manage the task events.
    Clock& clock;

    /// Next event in the clock queue. It points to an event with the same or a later start time than this event.
    EventImpl* next;

    /**
     * Previous event in the clock queue for a backward search in the clock queue. It points to an event with a earlier
     * start time. Events with a same start time have always an equal previous pointer.
     */
    EventImpl* previous;

    /// Area to protect access to event
    mutable Mutex mutex;

    /// Flag to indicate the tasking framework is inside the protected region. Only checked in stop.
    bool mutexLock;

    /**
     * Task specific processing of the time event.
     */
    void
    handle(void);

    /**
     * Configure the event to an periodic timing
     *
     * @param period Period time of the periodical clock. For a zero period a single shot is configured.
     *
     * @param offset Offset relative to the start time of the system. If the offset is in the past, the method
     * computes the next time point in the future by adding a multiple of the period to the offset. In case the
     * period is zero, the event start immediately.
     */
    void
    configurePeriodicTiming(const Time period, const Time offset);

    /**
     * Setter method for periodic schedule.
     * @param Reference to periodic schedule to set for the event.
     */
    void
    setPeriodicSchedule(PeriodicSchedule& schedule);
};

} // namespace Tasking

#endif /* INCLUDE_IMPL_TASKEVENT_IMPL_H_ */
