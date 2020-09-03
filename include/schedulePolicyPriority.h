/*
 * schedulePolicyPriority.h
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

#ifndef TASKING_INCLUDE_SCHEDULEPOLICYPRIORITY_H_
#define TASKING_INCLUDE_SCHEDULEPOLICYPRIORITY_H_

#include "schedulePolicy.h"
#include <mutex.h>

namespace Tasking
{

/// Priority based scheduling policy.
class SchedulePolicyPriority : public SchedulePolicy
{
public:
    /// Definition of a priority. Highest number has the highest priority.
    typedef unsigned int Priority;

    /// Initializer for the priority settings of a task
    struct Settings
    {
        /**
         * Initialize with a priority
         * @param Priority of the task
         */
        explicit Settings(Priority priority);

        /// Priority of the task
        Priority priority;
    };

    /**
     * Data to manage the queued tasks in a priority based discipline.
     */
    struct ManagementData : public SchedulePolicy::ManagementData
    {
        /// Initialize with initial data.
        ManagementData(Settings setting);

        /// Priority of the task.
        Settings settings;

        /// Pointer to a task with the next lower or equal priority.
        TaskImpl* next;
    };

    /// Initialize priority queue
    SchedulePolicyPriority(void);

    /**
     * Queue a task by priority in to the run queue. For tasks with the same priority the order is FIFO.
     * @param Reference to the task to queue in the run queue by its priority.
     * @return True when queue was empty at call time.
     */
    bool queue(TaskImpl& task) override;

    /**
     * Request and remove the next task according to the scheduling policy.
     * @return Pointer to the next task with the highest priority. If no task is available a NULL
     * pointer is returned.
     */
    TaskImpl* nextTask(void) override;

protected:
    /// Pointer to the head of the run queue. It is the task with the highest priority of tasks in the queue.
    TaskImpl* head;

    /// Mutex to protect access to run queue
    Mutex queueMutex;
};
} // namespace Tasking

#endif /* TASKING_INCLUDE_SCHEDULEPOLICYPRIORITY_H_ */
