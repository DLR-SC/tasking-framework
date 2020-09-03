/*
 * taskGroup_impl.h
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

#ifndef INCLUDE_IMPLGROUP_H_
#define INCLUDE_IMPLGROUP_H_

namespace Tasking {

class Task;
class TaskImpl;

/**
 * Implementation part of a Group.
 * @see Group
 */
struct GroupImpl
{
    /**
     * Initialize the implementation specific part of a group.
     *
     * @param n Number of tasks can bind to the task set. It is under response of the application programmer
     *            that no more calls to bindTo happens than specified by this parameter.
     *
     * @param taskList A list of task pointers
     */
    GroupImpl(unsigned int n, TaskImpl** p_taskList);

    /**
     * Request if all associated tasks has performed the method executed since the last reset of the group.
     *
     * @result true, if all tasks in the task list are executed since last reset operation.
     */
    bool
    areAllExecuted(void) const;

    /**
     * Reset all associated tasks. Activated but not yet started threads will not started after that call. It is
     * recommended to call reset from application code. To reset all joined tasks, it is recommended to do this with
     * the respective schedulers.
     */
    void
    reset(void);

    /**
     * The method is called by the scheduler when all tasks in the group has finalized.
     */
    void
    finalizeExecution(void);

    /**
     * List of associated tasks. Use join to associate tasks.
     * @see join
     */
    TaskImpl** taskList;

    /// Size of elements in the list of associated tasks.
    unsigned int maxTasks;
};

} // namespace Tasking

#endif /* INCLUDE_IMPLGROUP_H_ */
