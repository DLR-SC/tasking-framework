/*
 * schedulePolicy.h
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

#ifndef TASKING_INCLUDE_SCHEDULEPOLICY_H_
#define TASKING_INCLUDE_SCHEDULEPOLICY_H_

namespace Tasking
{

class TaskImpl;

/**
 * Interface class of a scheduling policy. For the implementation of a new scheduling policy the two structures
 * and two methods have to be implemented by a specialization of this class.
 */
class SchedulePolicy
{
public:
    /**
     * Structure to initialize policies with settings for a task, e.g. the task priority for a priority based
     * scheduling policy. A specialization of this class has to provide the corresponding structure when
     * task settings are needed for the policy. It is used to initialize the management data of a task.
     * @see ManagementData
     */
    struct Settings
    {
    };

    /**
     * Structure for data used by the implementation. This data is held by each task. Typical data are for
     * example pointers between tasks to implement a run queue. It is initialized with task settings.
     * A specialization of a scheduling policy has to provide this data structure.
     * @see Settings
     */
    struct ManagementData
    {
    };

    /// Needed for virtual methods
    virtual ~SchedulePolicy(void)
    {
    }

    /**
     * Queue a task according to the policy into the run queue. An implementation of a scheduling policy must implement
     * this method. Each task provides the management data structure to provide the memory space for the scheduling
     * policy. The method is called when a task switches the state from wait to pending.
     * @param task Reference to the task to queue in the run queue by the scheduling policy
     * @return True when queue was empty at call time.
     * @see ManagementData
     */
    virtual bool queue(Tasking::TaskImpl& task) = 0;

    /**
     * Request and remove the next task in the scheduling order. An implementation of a scheduling policy has to provide
     * this method. The delivered task will switch from state pending to run.
     * @return Pointer to the next task in the order of the scheduling policy. If no pending task is available, a NULL
     * pointer is returned.
     */
    virtual Tasking::TaskImpl* nextTask(void) = 0;
};

} // namespace Tasking

#endif /* TASKING_INCLUDE_SCHEDULEPOLICY_H_ */
