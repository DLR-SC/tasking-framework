/*
 * clock.cpp
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

#include <impl/clock_impl.h>
#include <scheduler.h>
#include <taskEvent.h>

class ProtectedSchedulerAccess : public Tasking::Scheduler
{
public:
    using Tasking::Scheduler::signal;
};

Tasking::Clock::Clock(Tasking::Scheduler& pScheduler) :
    scheduler(pScheduler),
    inTimeQueueMutex(false),
    queueHead(NULL),
    queueTail(NULL)
{
}

//-------------------------------------

Tasking::Clock::~Clock(void)
{
}

//-------------------------------------

void
Tasking::Clock::startAt(EventImpl& event, const Time time)
{
    // Signaling only outside of critical section.
    bool shouldSignal = false;
    // Start of timer only outside of critical section to avoid deadlock between mutex of time queue and mutex of clock.
    bool shouldStartTimer = false;
    int64_t delay = 0u;

    // Modification on a still queued event can corrupt queuing.
    timeQueueMutexMutex.enter();
    timeQueueMutex.enter();
    inTimeQueueMutex = true;
    timeQueueMutexMutex.leave();

    // Queue only if not queued yet.
    if (!event.queued)
    {
        // Set the correct next activation time
        event.nextActivation_ms = time;
        // Compute the new delay time for the timer. Usage of integer to handle past times implicit.
        delay = static_cast<int64_t>(time) - static_cast<int64_t>(getTime());

        // Take over into clock queue when delay is at least one millisecond, else schedule event
        if (delay > 0u)
        {
            // Operations on the queues is concurrent to clock thread
            // Enqueue in active list
            if (enqueue(event))
            {
                // And start it when enqueue replace the head element outside of protected region.
                shouldStartTimer = true;
            }
        }
        else
        {
            enqueueHead(event);
            shouldSignal = true;
        }
    }

    timeQueueMutexMutex.enter();
    inTimeQueueMutex = false;
    timeQueueMutex.leave();
    timeQueueMutexMutex.leave();

    // If necessary, signal the scheduler to execute handle methods of pending events
    if (shouldSignal)
    {
        static_cast<ProtectedSchedulerAccess*>(&scheduler)->signal();
    }
    // If necessary, start the timer because wake up time has changed.
    // Condition (shouldStartTimer && (delay > 0)) is always fulfilled
    if (shouldStartTimer)
    {
        startTimer(delay);
    }
}

//-------------------------------------

void
Tasking::Clock::startIn(EventImpl& event, const Time time)
{
    // If a time is given compute absolute starting time and enqueue, else enqueue as head with current time
    if (time != 0u)
    {
        startAt(event, getTime() + time);
    }
    else
    {
        // Signal only if also queued. By default assume no queuing.
        bool shouldSignal = false;

        // Modification on a still queued event can corrupt queuing.
        timeQueueMutexMutex.enter();
        timeQueueMutex.enter();
        inTimeQueueMutex = true;
        timeQueueMutexMutex.leave();

        // Only modify and enqueued event when it is not queued yet.
        if (!event.queued)
        {
            event.nextActivation_ms = getTime();
            enqueueHead(event);
            // After leaving protected area signal queuing.
            shouldSignal = true;
        }

        timeQueueMutexMutex.enter();
        inTimeQueueMutex = false;
        timeQueueMutex.leave();
        timeQueueMutexMutex.leave();

        if (shouldSignal)
        {
            static_cast<ProtectedSchedulerAccess*>(&scheduler)->signal();
        }
    }
}

//-------------------------------------

bool
Tasking::Clock::enqueue(EventImpl& event)
{
    // Only called by startAt, so always inside protected area of timeQueueMutex

    bool headReplaced = false;

    event.queued = true;
    if (queueHead == NULL)
    {
        // Nothing in the queue
        headReplaced = true;
        event.next = NULL;
        event.previous = NULL;
        queueHead = &event;
        queueTail = &event;
    }
    else
    {
        // Search position in the queue
        EventImpl* previousEvent = queueTail;
        // Queue is ordered by time all and we should found an element with smaller or equal time than the new element
        while ((previousEvent != NULL) && (previousEvent->nextActivation_ms > event.nextActivation_ms))
        {
            // Continue search with next previous time
            previousEvent = previousEvent->previous;
        }
        // Place in queue is found, element start time is smaller or equal to new element.
        // Is it the head element
        if (previousEvent == NULL)
        {
            // Replace head because we found the first element in the queue. All other times in queue are in the future
            headReplaced = true;
            // All elements at the head with the same tinme has at previous a nil pointer
            for (EventImpl* hasSameTime = queueHead; (hasSameTime != NULL) && (hasSameTime->previous == NULL);
                 hasSameTime = hasSameTime->next)
            {
                hasSameTime->previous = &event;
            }
            event.next = queueHead;
            event.previous = NULL;
            queueHead = &event;
        }
        else
        {
            // Search was stopped because start time of element is earlier or equal than element to queue
            // Previous should always point to an event with an earlier trigger time than the the new element
            if (previousEvent->nextActivation_ms == event.nextActivation_ms)
            {
                event.previous = previousEvent->previous;
            }
            else
            {
                event.previous = previousEvent;
            }
            // We now are between the found element and its next element
            event.next = previousEvent->next;
            previousEvent->next = &event;
            // If we found the last element in the queue we become the tail of the queue
            if (event.next == NULL)
            {
                // Tail of queue
                queueTail = &event;
            }
            else
            {
                // If not tail, the previous pointer should correct for all following elements with the same time
                Time nextTime = event.next->nextActivation_ms;
                for (EventImpl* correctPrevious = event.next;
                     (correctPrevious != NULL) && (correctPrevious->nextActivation_ms == nextTime); //
                     correctPrevious = correctPrevious->next)
                {
                    correctPrevious->previous = &event;
                }
            }
        }
    }
    return headReplaced;
}

//-------------------------------------

void
Tasking::Clock::enqueueHead(Tasking::EventImpl& event)
{
    // Only called by startAt or startIn, so always inside protected area of timeQueueMutex

    // If queue is not empty prepare head elements for enqueuing
    if (queueHead != NULL)
    {
        // If not start at the same time like the current head, than replace the previous pointer of block with same
        // start time
        if (event.nextActivation_ms < queueHead->nextActivation_ms)
        {
            for (EventImpl* queue = queueHead;
                 (queue != NULL) && (queue->nextActivation_ms == queueHead->nextActivation_ms); queue = queue->next)
            {
                queue->previous = &event;
            }
        }
    }
    // Prepare next and previous pointer of event to become new head element
    event.next = queueHead;
    event.previous = NULL;
    // Replace head element
    queueHead = &event;
}

//-------------------------------------

void
Tasking::Clock::dequeueAll(void)
{
    timeQueueMutex.enter();
    // Reset pointers of all events in queue
    EventImpl* event = queueHead;
    while (event != NULL)
    {
        EventImpl* next = event->next;
        event->queued = false;
        event->next = NULL;
        event->previous = NULL;
        event = next;
    }
    // Clear head and tail
    queueHead = NULL;
    queueTail = NULL;
    timeQueueMutex.leave();
}

//-------------------------------------

void
Tasking::Clock::dequeue(Tasking::EventImpl& event)
{
    // Flag to indicate if mutex was entered by this method
    bool enterMutex = false;

    // Enter protected area of time queue if not in currently
    timeQueueMutexMutex.enter();
    if (!inTimeQueueMutex)
    {
        timeQueueMutex.enter();
        enterMutex = true;
    }
    inTimeQueueMutex = true;
    timeQueueMutexMutex.leave();

    // If event queue is empty, do nothing
    if (queueHead != NULL)
    {
        // Find the element in the queue and it direct previous element
        EventImpl* current = queueHead;
        EventImpl* previous = NULL;
        while ((current != NULL) && (&event != current))
        {
            previous = current;
            current = current->next;
        }
        // Event is found in queue, so modify previous and next in queue
        if (current != NULL)
        {
            // Distinguish four cases. Element to remove is head, tail, head and tail, or inside queue
            if (current == queueTail)
            {
                // Current is tail
                if (current == queueHead)
                {
                    // Current is head and tail
                    queueHead = NULL;
                    queueTail = NULL;
                }
                else
                {
                    // Current is only tail
                    queueTail = previous; // Is in this case always not a null pointer
                    queueTail->next = NULL;
                }
            }
            else
            {
                // Current is not tail
                // Maybe we have to correct several previous pointer of events with same time
                if (current == queueHead)
                {
                    // Current is only head
                    queueHead = current->next; // Is in this case always not a null pointer
                }
                else
                {
                    // Current is inside queue, next pointer of the previous element must corrected
                    previous->next = current->next;
                }
                if (current->nextActivation_ms != current->next->nextActivation_ms)
                {
                    // Time of next differs, previous pointer must change of all following with the same time
                    for (EventImpl* hasSameTime = current->next;
                         (hasSameTime != NULL) &&
                         (hasSameTime->nextActivation_ms == current->next->nextActivation_ms); //
                         hasSameTime = hasSameTime->next)
                    {
                        hasSameTime->previous = previous;
                    }
                }
            }
        }
    }
    event.queued = false; // Mark as no longer queued
    event.next = NULL;
    event.previous = NULL;

    // Leave protected area if it was entered by this method
    timeQueueMutexMutex.enter();
    if (enterMutex)
    {
        inTimeQueueMutex = false;
        timeQueueMutex.leave();
    }
    timeQueueMutexMutex.leave();
}

//-------------------------------------

bool
Tasking::Clock::isEmtpy(void) const
{
    return (queueHead == NULL);
}

//-------------------------------------

bool
Tasking::Clock::isPending(void) const
{
    timeQueueMutex.enter();
    bool pends = (queueHead != NULL);
    if (pends)
    {
        pends = (queueHead->nextActivation_ms <= getTime());
    }
    timeQueueMutex.leave();
    return pends;
}

//-------------------------------------

Tasking::EventImpl*
Tasking::Clock::readFirstPending(void)
{
    EventImpl* result = NULL;
    // Working on clock queue is critical
    timeQueueMutex.enter();
    // Only remove when one is pending.
    if ((queueHead != NULL) && (queueHead->nextActivation_ms <= getTime()))
    {
        // One event is pending and this is the queue head by the sorting order.
        result = queueHead;
        // When head element available remove it from list
        queueHead = result->next;
        // Mark as no longer queued
        result->queued = false;
        // If queue gets empty tail must also corrected
        if (NULL == queueHead)
        {
            queueTail = NULL;
        }
        else
        {
            // For not empty queue new head element has no previous element.
            // If read event was not first of the block the previous pointer of events with same time must corrected.
            if (queueHead->previous != NULL)
            {
                for (EventImpl* hasSameTime = queueHead;
                     (hasSameTime != NULL) && (hasSameTime->nextActivation_ms == queueHead->nextActivation_ms);
                     hasSameTime = hasSameTime->next)
                {
                    hasSameTime->previous = NULL;
                }
            }
        }
    }
    timeQueueMutex.leave();

    return result;
}

//-------------------------------------

Tasking::Time
Tasking::Clock::getNextGapTime(void) const
{
    Time nextGapTime = 0u;
    // Search for gap time when the clock queue is not empty
    if (queueHead != NULL)
    {
        // Loop over the events in clock queue until a gap is found
        for (EventImpl* event = queueHead->next; event != NULL; event = event->next)
        {
            nextGapTime = event->nextActivation_ms - queueHead->nextActivation_ms;
            if (nextGapTime > 0u)
            {
                // Gap time is found, stop loop by jump to last event in queue.
                event = queueTail;
            }
        }
    }
    return nextGapTime;
}

//-------------------------------------

Tasking::Time
Tasking::Clock::getHeadTime(void) const
{
    Time headTime = 0u;
    EventImpl* head = queueHead;
    if (head != NULL)
    {
        headTime = head->nextActivation_ms;
    }
    return headTime;
}
