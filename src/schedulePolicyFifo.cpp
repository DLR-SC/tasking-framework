/*
 * schedulePolicyFifo.cpp
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

#include <cstddef>

#include <schedulePolicyFifo.h>
#include <task.h>

Tasking::SchedulePolicyFifo::ManagementData::ManagementData(void) : next(NULL)
{
}

// ----------------

Tasking::SchedulePolicyFifo::SchedulePolicyFifo(void) : head(NULL), tail(NULL)
{
}

// ----------------

bool
Tasking::SchedulePolicyFifo::queue(Tasking::TaskImpl& task)
{
    // Next element of enqueued task is always null
    static_cast<ManagementData*>(task.policyData)->next = NULL;

    queueMutex.enter();

    // Check if state is empty
    bool isEmpty = (head == NULL);
    if (isEmpty)
    { // When it is empty, than head should adjusted to new task
        head = &task;
    }
    else
    { // FIFO is not empty, correct old tail element
        static_cast<ManagementData*>(tail->policyData)->next = &task;
    }
    // New tail is always the task
    tail = &task;

    queueMutex.leave();
    return isEmpty;
}

// ----------------

Tasking::TaskImpl*
Tasking::SchedulePolicyFifo::nextTask(void)
{
    queueMutex.enter();

    // Get head element and correct it
    TaskImpl* next = head;
    if (head != NULL)
    {
        head = static_cast<ManagementData*>(head->policyData)->next;
        // When the FIFO becomes empty, tail must invalidated
        if (head == NULL)
        {
            tail = NULL;
        }
    }

    queueMutex.leave();
    return next;
}
