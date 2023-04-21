/*
 * schedulePolicyLifo.cpp
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

#include <schedulePolicyLifo.h>
#include <task.h>

Tasking::SchedulePolicyLifo::ManagementData::ManagementData(void) : next(NULL)
{
}

// ----------------

Tasking::SchedulePolicyLifo::SchedulePolicyLifo(void) : head(NULL)
{
}

// ----------------

bool
Tasking::SchedulePolicyLifo::queue(Tasking::TaskImpl& task)
{
    bool isEmpty = true;
    queueMutex.enter();
    // If not empty the next element copy head to new element and then replace head by new one
    if (head != NULL)
    {
        isEmpty = false;
        static_cast<ManagementData*>(task.policyData)->next = head;
    }
    head = &task;
    queueMutex.leave();
    return isEmpty;
}

// ----------------

Tasking::TaskImpl*
Tasking::SchedulePolicyLifo::nextTask(void)
{
    queueMutex.enter();
    TaskImpl* next = head;
    if (NULL != head)
    {
        head = static_cast<ManagementData*>(head->policyData)->next;
    }
    queueMutex.leave();
    return next;
}
