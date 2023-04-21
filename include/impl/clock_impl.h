/*
 * clock_impl.h
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

#ifndef TASKING_INCLUDE_CLOCK_H_
#define TASKING_INCLUDE_CLOCK_H_

#include "../taskEvent.h"
#include "../taskUtils.h"

namespace Tasking
{

class Scheduler;

/**
 * Base class to manage the start of events at a time point. It must be overloaded with a system specific clock
 * mechanism which trigger the scheduler for the execution of events at a specific time.
 */
class Clock
{
public:
    /**
     * Initialization of the clock and connect it to scheduler
     *
     * @param scheduler Reference to the schedule which should wake up in case of a clock event.
     */
    Clock(Scheduler& scheduler);

    /// Destructor
    virtual ~Clock(void);

    /**
     * Get the absolute time used to control events. The zero time depends on the bare metal implementation.
     * The method must be implemented by the bare metal implementation of the clock. Application programmer
     * can use this time for time stamps or to calculate the offset time of a periodic event.
     *
     * @result Time which is in the time frame used for triggering events in ms. Most of the time zero time is start
     * of the system.
     *
     * @see Event::setPeriodicTiming
     */
    virtual Time getTime(void) const = 0;

    /// @return True when no event is in the clock queue
    bool isEmtpy(void) const;

    /// @return True when activation time of the clock queue head element is equal or smaller than the current time.
    bool isPending(void) const;

    /**
     * Start an event at an absolute time.
     *
     * @param p_event Reference to the event to start at an absolute time
     * @param time Absolute time in ms when the event should started. Time zero depends on the bare metal
     * implementation. By default it should be the instantiation time of this class.
     */
    void startAt(EventImpl& p_event, const Time time);

    /**
     * Start an event at a relative time span from now.
     *
     * @param p_event Reference to the event to start at the relative time
     * @param time Relative time span from now in ms in which the event should started.
     */
    void startIn(EventImpl& p_event, const Time time);

    /**
     *  Enqueue an element to the clock queue. The method search the right position in the queue by the time,
     *  earliest time first. The last enqueued event is triggered first.
     *
     *  @param event Reference to the element to enqueue
     *
     *  @return True when the head element is replaced by the enqueued element, else false.
     */
    bool enqueue(EventImpl& event);

    /**
     * Replace directly the head of the clock queue without searching the correct spot. This is done with events
     * which has a delay time with zero or smaller.
     *
     * @param event Reference to the element which becomes the new head of the queue.
     */
    void enqueueHead(EventImpl& event);

    /**
     *  Dequeue an element from the queue.
     *  This method is used if an event is deleted to satisfy that the event will not triggered in the future. Such
     *  a trigger can lead into a memory corruption.
     *
     *  @param event Event to dequeue from the list.
     */
    void dequeue(EventImpl& event);

    /**
     * Remove all events from the clock queue.
     */
    void dequeueAll(void);

    /**
     * Stop a running timer and start the timer to wake up the system after a time span is over.
     * The method must override by the bare metal implementation
     *
     * @param timeSpan Length of the time interval. When the time interval pass, the system should wake up and trigger
     * the scheduler to handle pending time events.
     */
    virtual void startTimer(Time timeSpan) = 0;

    /**
     * Read and remove the first pending element from the clock queue.
     *
     * @return Pointer to the from the clock queue removed head element.
     */
    EventImpl* readFirstPending(void);

    /**
     *  @return The time of the first event in the queue with start time in the future.
     *  @see nonPendingHead
     */
    Time getNextStartTime(void);

    /**
     * @return Wake up time point of the clock queue head. If the clock queue is empty, the method return 0.
     */
    Time getHeadTime(void) const;

    /// Reference to the scheduler, which execute events from this clock implementation.
    Scheduler& scheduler;

    /// Mutex to protect the clock queue against concurrent access.
    mutable Mutex timeQueueMutex;

    /// Flag to indicate if still in mutex.
    volatile bool inTimeQueueMutex;

    /// Mutex to protect change of pair timeQueueMutex and inTimeQueueMutex.
    mutable Mutex timeQueueMutexMutex;

    /// Flag to indicate that a next pointer inside the clock queue was modified
    volatile bool nextInQueueModified;

    /**
     * Pointer to the clock queue head. This event has the earliest absolute wake up time or the same time like an
     * event with the same time queued first.
     */
    EventImpl* queueHead;

    /**
     * Pointer to the clock queue tail. This event has the highest absolute wake up time or an equal time to the
     * event enqueued after.
     */
    EventImpl* queueTail;

    /**
     * Point to the first event in the clock queue after request the next start time.
     * @see getNextStartTime
     */
    EventImpl* nonePendingHead;
};

} // namespace Tasking

#endif /* TASKING_INCLUDE_CLOCK_H_ */
