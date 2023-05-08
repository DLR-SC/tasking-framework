/*
 * task_Impl.h
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

#ifndef INCLUDE_IMPL_TASK_IMPL_H_
#define INCLUDE_IMPL_TASK_IMPL_H_

#include "../schedulePolicy.h"
#include "../taskInputArray.h"
#include "../taskUtils.h"

namespace Tasking
{

// Forward declarations
struct GroupImpl;
class Scheduler;

struct TaskImpl
{

    /**
     * First initialization step and connect the task to the scheduler. The task is not fully
     * initialized until the second initialization step with a call to construct is done.
     *
     * @param scheduler Reference to the scheduler. It provide means to execute this task.
     *
     * @param policy Reference to the memory for the scheduling policy. This structure is used
     * by the scheduler to manage the task related data according to the policy.
     *
     * @param task Reference to the task to which the implementation belongs to.
     *
     * @param inputArray Reference to an array of inputs associated with this task.
     *
     * @see taskId_t
     * @see construct
     */
    TaskImpl(Scheduler& scheduler, SchedulePolicy::ManagementData& policy, Task& task, InputArray& inputArray);

    /**
     * Activate the task if all inputs of the task activated. The method is called by input array when all inputs are
     * activated. Don't call it from application code except you want unexpected behavior.
     */
    void activate();

    /**
     * The method is called by the scheduler after synchronizationEnd is done and checks the connection to a task
     * group. If it is connected the finalization of the task is reported, if not the task is reset.
     */
    void finalizeExecution(void);

    /**
     * This method is called directly by the scheduler before the execution method is called. It loops over all inputs
     * to call synchronizeStart of all connected channels.
     *
     * @see Channel::synchronizeStart
     */
    void synchronizeStart(void);

    /**
     * This method is called by the scheduler directly after the execution methods end. It loops over all inputs to
     * call synchronizeEnd of all connected channels.
     *
     * @see Channel::synchronizeEnd
     */
    void synchronizeEnd(void);

    /**
     * Request if the task was executed when it is part of a group
     *
     * @result True, if the task was executed since the last call of reset for a group, else it is false.
     */
    bool isExecuted(void) const;

    /**
     * Enumeration for the different states of a task.
     */
    enum TaskState
    {
        /// The task is waiting until are all task inputs are activated.
        TASK_WAIT,
        /// The task is scheduled.
        TASK_RUN,
        /**
         * The task has finalize the run. This state will be active if the task join to a task group and the task is
         * waiting after its execution on the execution of the remaining tasks in the group.
         */
        TASK_FINISH,
        /// The task is currently inside the reset operation.
        TASK_RESET,
        /// An activation to the task is pending; all necessary inputs are activated
        TASK_PENDING
    };

    /// Reference to the task which is managed by this implementation class
    Task& parent;

    /**
     * State of the task.
     */
    volatile TaskState m_state;

    /// Array of all task inputs.
    InputArray& inputs;

    /// Helper for list of task at the scheduler. Normally this list is only needed for initialization
    TaskImpl* nextTaskAtScheduler;

    /// Reference to the associated scheduler which perform these task.
    Scheduler& associatedScheduler;

    /// Protection
    mutable Mutex taskMutex;

    /// Pointer to the data to manage the task according to the selected scheduling policy
    SchedulePolicy::ManagementData* policyData;

    /// Pointer to the group in which the task is joined.
    GroupImpl* group;
};

} // namespace Tasking

#endif /* INCLUDE_IMPL_TASK_IMPL_H_ */
