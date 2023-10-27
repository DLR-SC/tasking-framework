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
#include <taskUtils.h>

#include "accessor.h"

Tasking::Clock::Clock(Tasking::Scheduler& pScheduler) :
    scheduler(pScheduler), queueHead(nullptr), queueTail(nullptr), nonePendingHead(nullptr)
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
    {
        MutexGuard guard(timeQueueMutex);
        // Queue only if not queued yet.
        if (!event.queued)
        {
            // Set the correct next activation time
            event.nextActivation_ms = time;
            // Compute the new delay time for the timer. Usage of integer to handle past times implicit.
            Time currentTime = getTime();
            delay = static_cast<int64_t>(time) - static_cast<int64_t>(currentTime);

            // Take over into clock queue when delay is at least one millisecond, else schedule event
            if (delay > 0u)
            {
                // Enqueue in active list and start timer when event is the first future event
                shouldStartTimer = enqueue(currentTime, event);
            }
            else
            {
                // Start event immediately or start point where in the past.
                enqueueHead(event);
                shouldSignal = true;
            }
        }
    }

    // If necessary, signal the scheduler to execute handle methods of pending events
    if (shouldSignal)
    {
        TaskingAccessor().signal(scheduler);
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
        {
            MutexGuard guard(timeQueueMutex);

            // Only modify and enqueued event when it is not queued yet.
            if (!event.queued)
            {
                event.nextActivation_ms = getTime();
                enqueueHead(event);
                // After leaving protected area signal queuing.
                shouldSignal = true;
            }
        }

        if (shouldSignal)
        {
            TaskingAccessor().signal(scheduler);
        }
    }
}

//-------------------------------------

bool
Tasking::Clock::enqueue(Time currentTime, EventImpl& event)
{
    // Only called by startAt, so always inside protected area of timeQueueMutex

    bool firstFutureEvent = false;

    event.queued = true;
    event.next = nullptr;
    if (queueHead == nullptr)
    {
        // Nothing in the queue
        firstFutureEvent = true;
        event.previous = nullptr;
        queueHead = &event;
        queueTail = &event;
    }
    else
    {
        // Search position in the queue
        EventImpl* previousEvent = queueTail;
        // Queue is ordered by time all and we should found an element with smaller or equal time than the new element
        while ((previousEvent != nullptr) && (previousEvent->nextActivation_ms > event.nextActivation_ms))
        {
            // Continue search with next previous time
            previousEvent = previousEvent->previous;
        }
        // Place in queue is found, element start time is smaller or equal to new element.
        // Is it the head element
        if (previousEvent == nullptr)
        {
            // Replace head because we found the first element in the queue. All other times in queue are in the future
            firstFutureEvent = true;
            // All elements at the head with the same time has at previous a nil pointer
            for (EventImpl* hasSameTime = queueHead; (hasSameTime != nullptr) && (hasSameTime->previous == nullptr);
                 hasSameTime = hasSameTime->next)
            {
                hasSameTime->previous = &event;
            }
            event.next = queueHead;
            event.previous = nullptr;
            queueHead = &event;
            // When none pending event is set it need to be corrected
            if (nonePendingHead != nullptr)
            {
                nonePendingHead = queueHead;
            }
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
                // if the previous event is not in the future, we are the first future event
                if (previousEvent->nextActivation_ms <= currentTime)
                {
                    firstFutureEvent = true;
                }
            }
            // We now are between the found element and its next element
            event.next = previousEvent->next;
            previousEvent->next = &event;
            // Check if none pending event is the next one it has to be corrected
            if (event.next == nonePendingHead)
            {
                nonePendingHead = &event;
            }
            // If we found the last element in the queue we become the tail of the queue
            if (event.next == nullptr)
            {
                // Tail of queue
                queueTail = &event;
            }
            else
            {
                // If not tail, the previous pointer should correct for all following elements with the same time
                Time nextTime = event.next->nextActivation_ms;
                for (EventImpl* correctPrevious = event.next;
                     (correctPrevious != nullptr) && (correctPrevious->nextActivation_ms == nextTime); //
                     correctPrevious = correctPrevious->next)
                {
                    correctPrevious->previous = &event;
                }
            }
        }
    }

    return firstFutureEvent;
}

//-------------------------------------

void
Tasking::Clock::enqueueHead(Tasking::EventImpl& event)
{
    // Only called by startAt or startIn, so always inside protected area of timeQueueMutex

    // If queue is not empty prepare head elements for enqueuing
    if (queueHead != nullptr)
    {
        // If not start at the same time like the current head, than replace the previous pointer of block with same
        // start time
        if (event.nextActivation_ms < queueHead->nextActivation_ms)
        {
            for (EventImpl* queue = queueHead;
                 (queue != nullptr) && (queue->nextActivation_ms == queueHead->nextActivation_ms); queue = queue->next)
            {
                queue->previous = &event;
            }
        }
    }
    else
    {
        // No event in the queue, new event is also tail of queue
        queueTail = &event;
    }
    // Prepare next and previous pointer of event to become new head element
    event.next = queueHead;
    event.previous = nullptr;
    // Replace head element
    queueHead = &event;
}

//-------------------------------------

void
Tasking::Clock::dequeueAll(void)
{
    // Enter critical section to modify clock queue
    MutexGuard guard(timeQueueMutex);

    // No further event will fire so none pending head will not valid anymore
    nonePendingHead = nullptr;

    // Reset pointers of all events in queue
    EventImpl* event = queueHead;
    while (event != nullptr)
    {
        EventImpl* next = event->next;
        event->queued = false;
        event->next = nullptr;
        event->previous = nullptr;
        event = next;
    }
    // Clear head and tail
    queueHead = nullptr;
    queueTail = nullptr;
}

//-------------------------------------

void
Tasking::Clock::dequeue(Tasking::EventImpl& event)
{
    MutexGuard guard(timeQueueMutex);

    // If element is the first none pending event the hone pending head need replaced
    if (&event == nonePendingHead)
    {
        nonePendingHead = event.next;
    }

    // If event queue is empty, do nothing
    if (queueHead != nullptr)
    {
        // Find the element in the queue and it direct previous element
        EventImpl* current = queueHead;
        EventImpl* previous = nullptr;
        while ((current != nullptr) && (&event != current))
        {
            previous = current;
            current = current->next;
        }
        // Event is found in queue, so modify previous and next in queue
        if (current != nullptr)
        {
            // Distinguish four cases. Element to remove is head, tail, head and tail, or inside queue
            if (current == queueTail)
            {
                // Current is tail
                if (current == queueHead)
                {
                    // Current is head and tail
                    queueHead = nullptr;
                    queueTail = nullptr;
                }
                else
                {
                    // Current is only tail
                    queueTail = previous; // Is in this case always not a nullptr pointer
                    queueTail->next = nullptr;
                }
            }
            else
            {
                // Current is not tail
                // Maybe we have to correct several previous pointer of events with same time
                if (current == queueHead)
                {
                    // Current is only head
                    queueHead = current->next; // Is in this case always not a nullptr pointer
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
                         (hasSameTime != nullptr) &&
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
    event.next = nullptr;
    event.previous = nullptr;
}

//-------------------------------------

bool
Tasking::Clock::isEmtpy(void) const
{
    MutexGuard guard(timeQueueMutex);
    return (queueHead == nullptr);
}

//-------------------------------------

bool
Tasking::Clock::isPending(void) const
{
    MutexGuard guard(timeQueueMutex);
    bool pends = (queueHead != nullptr);
    if (pends)
    {
        pends = (queueHead->nextActivation_ms <= getTime());
    }
    return pends;
}

//-------------------------------------

Tasking::EventImpl*
Tasking::Clock::readFirstPending(void)
{
    EventImpl* result = nullptr;

    // Working on clock queue is critical
    MutexGuard guard(timeQueueMutex);
    // Only remove when one is pending.
    if ((queueHead != nullptr) && (queueHead->nextActivation_ms <= getTime()))
    {
        // If element is the first none pending event the hone pending head need replaced
        if (queueHead == nonePendingHead)
        {
            nonePendingHead = queueHead->next;
        }

        // One event is pending and this is the queue head by the sorting order.
        result = queueHead;
        // When head element available remove it from list
        queueHead = result->next;
        // Mark as no longer queued
        result->queued = false;
        result->next = nullptr;
        // If queue gets empty tail must also corrected
        if (nullptr == queueHead)
        {
            queueTail = nullptr;
        }
        else
        {
            // For not empty queue new head element has no previous element.
            // If read event was not first of the block the previous pointer of events with same time must corrected.
            if (queueHead->previous != nullptr)
            {
                for (EventImpl* hasSameTime = queueHead;
                     (hasSameTime != nullptr) && (hasSameTime->nextActivation_ms == queueHead->nextActivation_ms);
                     hasSameTime = hasSameTime->next)
                {
                    hasSameTime->previous = nullptr;
                }
            }
        }
    }

    return result;
}

//-------------------------------------

Tasking::Time
Tasking::Clock::getNextStartTime(void)
{
    Time nextStartTime = 0u;
    Time currentTime = getTime();
    EventImpl* searchEvent = nullptr;

    // Search first event in the future. Start with the last one found by getNextStartTime.
    searchEvent = nonePendingHead;
    if (searchEvent == nullptr)
    {
        searchEvent = queueHead;
    }
    // Search go until end of queue or when a new start time is found
    while ((searchEvent != nullptr) && (nextStartTime == 0u))
    {
        // Is search event pending in the past or now
        if (searchEvent->nextActivation_ms <= currentTime)
        {
            // Is in past or now, check the next one
            searchEvent = searchEvent->next;
            // Before utilize search event it check for queue modification
        }
        else
        {
            // Search event will start in the future, next wake up time in queue found
            nextStartTime = searchEvent->nextActivation_ms;
        }
    }
    // Mark for next search the starting point in queue.
    // When loop ended by null pointer also reset of pending head happen.
    nonePendingHead = searchEvent;

    return nextStartTime;
}

//-------------------------------------

Tasking::Time
Tasking::Clock::getHeadTime(void) const
{
    Time headTime = 0u;
    EventImpl* head = queueHead;
    if (head != nullptr)
    {
        headTime = head->nextActivation_ms;
    }
    return headTime;
}
