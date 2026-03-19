/*
 * schedulePolicyPSlot.cpp
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

#include <schedulePolicyPSlot.h>
#include <task.h>

Tasking::SchedulePolicyPSlot::FifoSlot::FifoSlot(void) : head(nullptr), tail(nullptr)
{
}

Tasking::SchedulePolicyPSlot::Settings::Settings(Priority p_priority) : priority(p_priority)
{
}

// ----------------

Tasking::SchedulePolicyPSlot::ManagementData::ManagementData(Settings setting) :
    next(nullptr), priority(setting.priority)
{
}

// ----------------

Tasking::SchedulePolicyPSlot::SchedulePolicyPSlot(FifoSlot* slotMemory, unsigned int numberOfSlots) :
    prioritySlots(slotMemory), highestPriorSlot(numberOfSlots), maxPrioritySlot(numberOfSlots)
{
}

// ----------------

bool
Tasking::SchedulePolicyPSlot::queue(Tasking::TaskImpl& task)
{
    // Next element of enqueued task is always nullptr
    static_cast<ManagementData*>(task.policyData)->next = nullptr;

    // Get the priority and limit to the maximum priority
    unsigned int priority = static_cast<SchedulePolicyPSlot::ManagementData*>(task.policyData)->priority;
    if (priority >= maxPrioritySlot)
    {
        priority = maxPrioritySlot - 1u;
    }

    MutexGuard guard(queueMutex);

    // Get return value if all slots are empty
    bool isEmpty = (highestPriorSlot == maxPrioritySlot);

    // Adjust slot for the next task is necessary
    if (isEmpty || (priority > highestPriorSlot))
    {
        highestPriorSlot = priority;
    }

    // Check if priority slot is empty
    if (nullptr == prioritySlots[priority].head)
    { // When FIFO is empty, then head should be adjusted to new task
        prioritySlots[priority].head = &task;
    }
    else
    { // FIFO in slot is not empty. The task becomes next of current tail
        static_cast<ManagementData*>(prioritySlots[priority].tail->policyData)->next = &task;
    }
    // New tail is always the task itself
    prioritySlots[priority].tail = &task;

    return isEmpty;
}

// ----------------

Tasking::TaskImpl*
Tasking::SchedulePolicyPSlot::nextTask(void)
{
    MutexGuard guard(queueMutex);

    // Get head element from highest prioritized slot and adjust highest prioritized slot if slot becomes empty.
    TaskImpl* result = nullptr;
    if (highestPriorSlot < maxPrioritySlot)
    {
        result = prioritySlots[highestPriorSlot].head;
        prioritySlots[highestPriorSlot].head = static_cast<ManagementData*>(result->policyData)->next;
        // When the FIFO in slot becomes empty, tail must invalidated and the next slot with a task need to be found
        if (prioritySlots[highestPriorSlot].head == nullptr)
        {
            prioritySlots[highestPriorSlot].tail = nullptr;
            while ((highestPriorSlot < maxPrioritySlot) && prioritySlots[highestPriorSlot].head == nullptr)
            {
                // Stop loop when no task is waiting in the lowest priority slot.
                if (highestPriorSlot == 0u)
                {
                    highestPriorSlot = maxPrioritySlot;
                }
                else
                {
                    --highestPriorSlot;
                    // Underflow hold (nextSlot < maxPrioritySlot) to terminate loop and mark as empty
                }
            }
        }
    }

    return result;
}
