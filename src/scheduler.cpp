/*
 * scheduler.cpp
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

#include <scheduler.h>
#include <task.h>

#include "accessor.h"

Tasking::Scheduler::Scheduler(SchedulePolicy& schedulePolicy, Clock& clock) : impl(*this, schedulePolicy, clock)
{
    // Nothing else to do
}

// ------------------------------------

Tasking::Scheduler::~Scheduler(void)
{
    // Only necessary because class has virtual methods
}

// ------------------------------------

void
Tasking::Scheduler::start(bool doReset)
{
    // Set in running mode
    impl.running = true;

    if (doReset)
    {
        // Reset all associated tasks
        for (TaskImpl* task = impl.associatedTasks; (task != nullptr); task = task->nextTaskAtScheduler)
        {
            // Doing a reset if option is set
            task->parent.reset();
        }
    }
    else
    { // No reset is performed, for this case check activations state of tasks and start them if activated
        for (TaskImpl* task = impl.associatedTasks; (task != nullptr); task = task->nextTaskAtScheduler)
        {
            // If reset option is not set, check if tasks are activated and start them
            if (task->inputs.isActivated())
            {
                impl.perform(*task);
            }
        }
    }
}

// ------------------------------------

void
Tasking::Scheduler::terminate(bool doNotRemovePendingTasks)
{
    impl.running = false;
    // If clean up is needed read tasks from run queue until it is empty
    impl.clock.dequeueAll();
    while (!doNotRemovePendingTasks && (nullptr != impl.policy.nextTask()))
        ;
    // Wait until running tasks are terminated.
    waitUntilEmpty();
}

// ------------------------------------

void
Tasking::Scheduler::initialize(void)
{
    for (TaskImpl* task = impl.associatedTasks; task != nullptr; task = task->nextTaskAtScheduler)
    {
        TaskingAccessor().initialize(task->parent);
    }
}

// ====================================

Tasking::SchedulerImpl::SchedulerImpl(Scheduler& scheduler, SchedulePolicy& schedulePolicy, Clock& p_clock) :
    parent(scheduler), policy(schedulePolicy), associatedTasks(nullptr), clock(p_clock), running(false)
{
    // Nothing else to do
}

// ------------------------------------

void
Tasking::SchedulerImpl::add(Tasking::TaskImpl& task)
{
    task.nextTaskAtScheduler = associatedTasks;
    associatedTasks = &task;
}

// ------------------------------------

void
Tasking::SchedulerImpl::perform(Tasking::TaskImpl& task)
{
    // Do only something when the scheduler is running.
    if (running)
    {
        // Queue task for execution and signal scheduler execution model
        policy.queue(task);
        TaskingAccessor().signal(parent);
    }
}

// ------------------------------------

void
Tasking::SchedulerImpl::handleEvents(void)
{
    EventImpl* event = clock.readFirstPending();
    while (event != nullptr)
    {
        event->handle();
        event = clock.readFirstPending();
    }
}

// ------------------------------------

void
Tasking::SchedulerImpl::execute(Tasking::TaskImpl& task) const
{
    TaskImpl* uptask = &task;
    synchronizationMutex.enter();
    uptask->synchronizeStart();
    synchronizationMutex.leave();
    TaskingAccessor().execute(uptask->parent);
    synchronizationMutex.enter();
    uptask->synchronizeEnd();
    uptask->finalizeExecution();
    synchronizationMutex.leave();
}
