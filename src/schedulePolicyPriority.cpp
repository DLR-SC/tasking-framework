/*
 * schedulePolicyPriority.cpp
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

#include <schedulePolicyPriority.h>
#include <task.h>

Tasking::SchedulePolicyPriority::Settings::Settings(Priority p_priority) : priority(p_priority)
{
}

// ----------------

Tasking::SchedulePolicyPriority::ManagementData::ManagementData(Settings setting) : settings(setting), next(nullptr)
{
}

// ----------------

Tasking::SchedulePolicyPriority::SchedulePolicyPriority(void) : head(nullptr)
{
}

// ----------------

bool
Tasking::SchedulePolicyPriority::queue(Tasking::TaskImpl& task)
{
    bool isEmpty = true;
    // Simplify access to task data by cast it now to matching type
    ManagementData* taskData = static_cast<ManagementData*>(task.policyData);
    queueMutex.enter();

    // Check if queue is empty
    if (head != nullptr)
    { // Non empty queue
        isEmpty = false;
        // Check if head element must replaced
        if (taskData->settings.priority > static_cast<ManagementData*>(head->policyData)->settings.priority)
        {
            taskData->next = head;
            head = &task;
        }
        else
        { // Search position in the queue
            TaskImpl* previous = head;
            TaskImpl* next = static_cast<ManagementData*>(previous->policyData)->next;
            while ((next != nullptr) &&
                   (taskData->settings.priority <= static_cast<ManagementData*>(next->policyData)->settings.priority))
            {
                previous = next;
                next = static_cast<ManagementData*>(next->policyData)->next;
            }
            // Put task between previous and next task
            taskData->next = next;
            static_cast<ManagementData*>(previous->policyData)->next = &task;
        }
    }
    else
    { // Empty queue
        taskData->next = nullptr;
        head = &task;
    }

    queueMutex.leave();
    return isEmpty;
}

// ----------------

Tasking::TaskImpl*
Tasking::SchedulePolicyPriority::nextTask(void)
{
    queueMutex.enter();

    // Get head element and correct it
    TaskImpl* next = head;
    if (head != nullptr)
    {
        head = static_cast<ManagementData*>(head->policyData)->next;
    }

    queueMutex.leave();
    return next;
}
