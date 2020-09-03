/*
 * taskGroup.cpp
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

#include <taskGroup.h>
#include <task.h>

Tasking::Group::Group(unsigned int n, TaskImpl** p_taskList) : impl(n, p_taskList)
{
}

//-------------------------------------

bool
Tasking::Group::isValid() const
{
    // Check if each slot in the task list is filled and the tasks of the group are valid.
    bool valid = true;
    for (unsigned int i = 0u; valid && (i < impl.maxTasks); ++i)
    {
        valid = (impl.taskList[i] != nullptr);
        if (valid)
        {
            valid = impl.taskList[i]->parent.isValid();
        }
    }
    return valid;
}

//-------------------------------------

void
Tasking::Group::join(Tasking::Task& task)
{
    // Find a not occupied position in the task list
    unsigned int freeSlot = 0;
    while ((freeSlot < impl.maxTasks) && (impl.taskList[freeSlot] != NULL))
    {
        ++freeSlot;
    }
    // When a free slot is found, put pointer of task to the list
    if (freeSlot < impl.maxTasks)
    {
        TaskImpl* taskImpl = &task.joinTo(impl);
        impl.taskList[freeSlot] = taskImpl;
    }
}

// ====================================

Tasking::GroupImpl::GroupImpl(unsigned int n, TaskImpl** p_taskList) : taskList(p_taskList), maxTasks(n)
{
    // Set all pointer to zero
    for (unsigned int i = 0u; i < maxTasks; ++i)
    {
        taskList[i] = nullptr;
    }
}

//-------------------------------------

void
Tasking::GroupImpl::finalizeExecution(void)
{
    if (areAllExecuted())
    {
        reset();
    }
}

//-------------------------------------

bool
Tasking::GroupImpl::areAllExecuted(void) const
{
    bool result = true;
    for (unsigned int i = 0; result && (i < maxTasks) && (taskList[i] != NULL); i++)
    {
        result = taskList[i]->isExecuted();
    }
    return result;
}

//-------------------------------------

void
Tasking::GroupImpl::reset(void)
{
    // Reset all tasks of the group;
    for (unsigned int i = 0; (i < maxTasks) && (taskList[i] != NULL); i++)
    {
        taskList[i]->parent.reset();
    }
}
