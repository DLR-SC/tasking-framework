/*
 * schedulePolicyLifo.h
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

#ifndef TASKING_INCLUDE_SCHEDULEPOLICYLIFO_H_
#define TASKING_INCLUDE_SCHEDULEPOLICYLIFO_H_

#include "schedulePolicy.h"
#include <mutex.h>

namespace Tasking
{

/// Manage run queue in "Last In, First Out " order.
class SchedulePolicyLifo : public SchedulePolicy
{
public:
    /**
     * Data to manage the queued tasks in a LIFO discipline.
     */
    struct ManagementData : public SchedulePolicy::ManagementData
    {
    public:
        /// Initialize queue with zero data
        ManagementData(void);

        /// Pointer to the next task in LIFO order. It will be scheduled before the current task.
        TaskImpl* next;
    };

    /// Null initialization of LIFO queue.
    SchedulePolicyLifo(void);

    /**
     * Queue a task in LIFO order to the run queue
     * @param task Reference to the task to queue into the run queue.
     * @return True when queue is empty at call time.
     */
    bool queue(TaskImpl& task) override;

    /**
     * Request and remove the next task in LIFO order.
     * @return Pointer to the next task following the LIFO order. If no task is available a NULL
     * pointer is returned.
     */
    TaskImpl* nextTask(void) override;

protected:
    /// Pointer to the last queued task in the LIFO.
    TaskImpl* head;

    /// Mutex to protect access to run queue
    Mutex queueMutex;
};
} // namespace Tasking

#endif /* TASKING_INCLUDE_SCHEDULEPOLICYLIFO_H_ */
