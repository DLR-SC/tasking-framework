/*
 * taskGroup.h
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

#ifndef TASKGROUP_H_
#define TASKGROUP_H_

#include "impl/taskGroup_impl.h"

namespace Tasking
{

class Task;

/**
 * With a group several tasks can be combined in such a way, that a task can only restart when all other
 * tasks in the group have finalized their execution. In detail, the reset behavior for the tasks is different.
 * A task outside of a group is reset by the scheduler directly after its run. If the task is part of a group,
 * than the task is reset by the scheduler after all tasks, which are part of the group, finalized their run.
 *
 * It is recommend to instantiate a group with the template class GroupProvider.
 *
 * @see GroupProvider
 */
class Group
{
public:
    /**
     * @return True if all task have joined to the task group and all of these tasks are valid.
     */
    bool isValid(void) const;

    /**
     * Associate a task to the task group. If more Tasks join the group than the group can manage, the task is
     * not added to the group. Leaving a group is not foreseen.
     *
     * @param task Reference to the task joining the task list.
     */
    void join(Task& task);

protected:
    /**
     * Initialize the group.
     *
     * @param n Number of tasks can bind to the task group. It is under response of the application programmer,
     *            that no more calls to bindTo happens than specified by this parameter.
     *
     * @param p_taskList A list of task pointers
     */
    Group(unsigned int n, TaskImpl** p_taskList);

    /**
     * Request if all associated tasks have performed the method execute since the last reset of the group.
     *
     * @result true, if all tasks in the task list have been executed since the last reset operation.
     */
    bool areAllExecuted(void) const;

private:
    /// Implementation specific details of group.
    GroupImpl impl;
};

/**
 * Template class to set up the class Group.
 *
 * @tparam size Maximum number of tasks the group is able to manage.
 */
template<unsigned int size>
class GroupProvider : public Group
{
public:
    /// Set up the group of tasks and initialize it.
    GroupProvider(void);

private:
    /// Memory to manage the task which are joined to the group.
    TaskImpl* taskList[size];
};

// ----- inlines -----

template<unsigned int size>
GroupProvider<size>::GroupProvider(void) : Group(size, taskList)
{
}

inline bool
Group::areAllExecuted(void) const
{
    return impl.areAllExecuted();
}

} // namespace Tasking

#endif /* TASKGROUP_H_ */
