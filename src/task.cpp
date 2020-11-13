/*
 * task.cpp
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

#include <task.h>
#include <taskInput.h>
#include <taskGroup.h>
#include <scheduler.h>

// Need access to protected elements of scheduler and group. Protection is only valid for application code
class ProtectedSchedulerAccess : public Tasking::Scheduler
{
public:
    using Tasking::Scheduler::getImpl;
};
class ProtectedInputAccess : public Tasking::Input
{
public:
    using Tasking::Input::synchronizeEnd;
    using Tasking::Input::synchronizeStart;
};

Tasking::Task::Task(Scheduler& scheduler, SchedulePolicy::ManagementData& policy, InputArray& inputArray,
                    TaskId taskId) :
    m_taskId(taskId), impl(scheduler, policy, *this, inputArray)
{
    static TaskId autoId = 1u;
    if (taskId == 0)
    {
        m_taskId = autoId;
        ++autoId;
    }
}

//-------------------------------------

Tasking::Task::Task(Scheduler& scheduler, SchedulePolicy::ManagementData& policy, InputArray& inputArray,
                    const char* taskName) :
    Task(scheduler, policy, inputArray, getTaskIdFromName(taskName))
{
}

//-------------------------------------

Tasking::Task::~Task(void)
{
}

//-------------------------------------

void
Tasking::Task::construct(void)
{
    // Connect task to all inputs in the input array
    impl.inputs.connectTask(impl);
}

//-------------------------------------

bool
Tasking::Task::configureInput(unsigned int key, Channel& channel)
{
    return impl.inputs[key].associate(channel);
}

//-------------------------------------

bool
Tasking::Task::isValid(void) const
{
    return impl.inputs.isValid();
}

//-------------------------------------

void
Tasking::Task::initialize(void)
{
    // Nothing to do by default.
}

//-------------------------------------

void
Tasking::Task::reset(void)
{
    impl.taskMutex.enter();
    impl.m_state = TaskImpl::TASK_RESET;
    impl.taskMutex.leave();
    impl.inputs.reset();
    impl.taskMutex.enter();
    if ((impl.m_state == TaskImpl::TASK_PENDING) && impl.inputs.isActivated())
    {
        impl.m_state = TaskImpl::TASK_RUN;
        static_cast<ProtectedSchedulerAccess&>(impl.associatedScheduler).getImpl().perform(impl);
    }
    else
    {
        impl.m_state = TaskImpl::TASK_WAIT;
    }
    impl.taskMutex.leave();
}

//-------------------------------------

Tasking::TaskId
Tasking::Task::getTaskId(void) const
{
    return m_taskId;
}

//-------------------------------------

void
Tasking::Task::setTaskName(const char* newTaskName)
{
    m_taskId = getTaskIdFromName(newTaskName);
}

//-------------------------------------

void
Tasking::Task::setTaskId(TaskId newTaskId)
{
    m_taskId = newTaskId;
}

// ====================================

Tasking::TaskImpl::TaskImpl(Scheduler& scheduler, SchedulePolicy::ManagementData& policy, Task& task,
                            InputArray& inputArray) :
    parent(task),
    m_state(TASK_FINISH),
    inputs(inputArray),
    nextTaskAtScheduler(NULL),
    associatedScheduler(scheduler),
    policyData(&policy),
    group(NULL)
{
    static_cast<ProtectedSchedulerAccess&>(scheduler).getImpl().add(*this);
}

//-------------------------------------

void
Tasking::TaskImpl::activate(void)
{

    taskMutex.enter();

    // if the task is already PENDING, we should do nothing here; especially, not entering the monitor
    if (m_state != TaskImpl::TASK_PENDING)
    {
        // Condition only true if a reset on this task is currently active.
        // By the reset we are inside the monitor. Other tasks are outside of state task reset.
        if (m_state == TaskImpl::TASK_RESET)
        {
            if (inputs.isActivated())
            {
                m_state = TaskImpl::TASK_PENDING;
            }
        }
        else
        {
            // Not in state TASK_PENDING or TASK_RESET
            if (m_state == TaskImpl::TASK_WAIT)
            {
                // Search if all inputs are active or activate if final input was triggered
                if (inputs.isActivated())
                {
                    // Initiate the execution
                    m_state = TaskImpl::TASK_RUN;
                    static_cast<ProtectedSchedulerAccess&>(associatedScheduler).getImpl().perform(*this);
                }
            }
        }
    }
    taskMutex.leave();
}

//-------------------------------------

Tasking::TaskImpl&
Tasking::Task::joinTo(GroupImpl& group)
{
    impl.group = &group;
    return impl;
}

//-------------------------------------

void
Tasking::TaskImpl::finalizeExecution(void)
{
    // If is part of no group, do a direct reset, else finalize the group.
    if (group == NULL)
    {
        parent.reset();
    }
    else
    {
        // Possible running condition to activate when the state is changed.
        taskMutex.enter();
        m_state = TASK_FINISH;
        taskMutex.leave();
        group->finalizeExecution();
    }
}

//-------------------------------------

void
Tasking::TaskImpl::synchronizeStart(void)
{
    for (unsigned int i = 0; (i < inputs.size()); i++)
    {
        static_cast<ProtectedInputAccess&>(inputs[i]).synchronizeStart();
    }
}

//-------------------------------------

void
Tasking::TaskImpl::synchronizeEnd(void)
{
    for (unsigned int i = 0; (i < inputs.size()); i++)
    {
        static_cast<ProtectedInputAccess&>(inputs[i]).synchronizeEnd();
    }
}

//-------------------------------------

bool
Tasking::TaskImpl::isExecuted(void) const
{
    taskMutex.enter();
    bool executionState = (m_state == TASK_FINISH);
    taskMutex.leave();
    return executionState;
}
