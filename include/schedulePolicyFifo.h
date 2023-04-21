/*
 * schedulePolicyFifo.h
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

#ifndef OTASKING_INCLUDE_SCHEDULEPOLICYFIFO_H_
#define OTASKING_INCLUDE_SCHEDULEPOLICYFIFO_H_

#include "schedulePolicy.h"
#include "taskUtils.h"

namespace Tasking
{

/**
 * Scheduling policy "First in, first out"
 */
class SchedulePolicyFifo : public SchedulePolicy
{
public:
    /**
     * Data to manage the queued tasks in a FIFO discipline.
     */
    struct ManagementData : public SchedulePolicy::ManagementData
    {
    public:
        /// Initialize with zero data
        ManagementData(void);

        /// Pointer to the next task in the FIFO queue. It will be scheduled after the current task.
        TaskImpl* next;
    };

    /// Null initialization of head and tail element.
    SchedulePolicyFifo(void);

    /**
     * Put a task at the tail of the FIFO queue.
     * @param task Reference to the task, which will queued in FIFO order to the run queue
     * @return True when queue was empty at call
     */
    bool queue(TaskImpl& task) override;

    /**
     * Request and remove the next task according to the scheduling policy.
     * @return Pointer to the head element of the FIFO at call time. If no task is available a NULL
     * pointer is returned.
     */
    Tasking::TaskImpl* nextTask(void) override;

protected:
    /// Pointer to the first queued task in the FIFO or null if FIFO is empty.
    TaskImpl* head;

    /// Pointer to the last queued task in the FIFO. If queue is empty, the value is not valid.
    TaskImpl* tail;

    /// Mutex to protect access to run queue
    Mutex queueMutex;
};
} // namespace Tasking

#endif /* TASKING_INCLUDE_SCHEDULEPOLICYFIFO_H_ */
