/*
 * scheduler_impl.h
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

#ifndef TASKING_INCLUDE_IMPL_SCHEDULER_IMPL_H_
#define TASKING_INCLUDE_IMPL_SCHEDULER_IMPL_H_

#include "schedulePolicy.h"
#include "impl/clock_impl.h"
#include <mutex.h>

namespace Tasking
{

// Forward name declarations
class TaskImpl;
class Scheduler;

/**
 * Common interface to the scheduler used by the Tasking Framework elements. It is recommended to use the template
 * class SchedulerProvider to instantiate a scheduler.
 * @see SchedulerProvider
 */
struct SchedulerImpl
{
    /**
     * Initialize the scheduler.
     *
     * @param scheduler Reference to the scheduler which is implementation by the structure.
     * @param schedulePolicy Reference to the used scheduling policy for the scheduler.
     * @param clock Reference to the clock used by the scheduler implementation
     */
    SchedulerImpl(Scheduler& scheduler, SchedulePolicy& schedulePolicy, Clock& clock);

    /**
     * Adding a task to the internal list of associated tasks. This method is called by the constructor of the task.
     */
    void add(TaskImpl& task);

    /**
     * Initiate the execution of the referenced task. By default the state of the task switch to pending by this call.
     * The exact starting time for the execution depends on the selected schedule policy and the number of available
     * executors. If the scheduler is not started or terminated, a call has no effect.
     *
     * @param task Reference of the task, which execution should initiated.
     */
    void perform(TaskImpl& task);

    /**
     * Iterate over all pending events and execute them until no further event is pending. The method
     * should call by the scheduler implementation frequently.
     */
    void handleEvents(void);

    /**
     * Method which is called by the scheduler implementation to execute a task. The method embed the task
     * execution inside the synchronization call and finalize the task execution.
     */
    void execute(TaskImpl& task) const;

    /// Reference to scheduler which is implementation by this structure.
    Scheduler& parent;

    /// Reference to the schedule policy used by the scheduler.
    SchedulePolicy& policy;

    /// Pointer to the list of tasks associated to this scheduler. Activations of these tasks will handled by this
    /// scheduler.
    TaskImpl* associatedTasks;

    /// Reference to the clock used by the scheduler implementation.
    Clock& clock;

    /**
     *  FLag to indicate if the scheduler is running or not. The flag is checked whenever a task should perform.
     *  @see perform
     *  @see start
     *  @see terminate
     */
    bool running;

    /**
     * Mutex for save synchronization calls at task executor level. The mutex is used by the method execute.
     * @see execute
     */
    mutable Mutex synchronizationMutex;
};

} // namespace Tasking

#endif /* TASKING_INCLUDE_IMPL_SCHEDULER_IMPL_H_ */
