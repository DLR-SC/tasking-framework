/*
 * schedulerExecutionModel.cpp
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

#include "schedulerExecutionModel.h"
#include <taskingConfig.h>

Tasking::SchedulerExecutionModel::Executor::Executor(void) :
    Thread(executorPriority, executorStackSize, "TSKE"),
    running(false),
    schedulerModel(nullptr),
    schedulerImpl(nullptr),
    nextFree(nullptr)
{
}

// ----------------

Tasking::SchedulerExecutionModel::Executor::~Executor(void)
{
    // Nothing else to do
}

// ----------------

void
Tasking::SchedulerExecutionModel::Executor::startExecutor(SchedulerExecutionModel& scheduler)
{
    // Hand over pointer to scheduler
    schedulerModel = &scheduler;
    schedulerImpl = &scheduler.getImpl();
    // And start the thread
    start();
}

// ----------------

void
Tasking::SchedulerExecutionModel::Executor::run(void)
{
    // Go inside critical section for start up of thread
    signaler.enter();

    // Mark as running, later wait will leave signaler
    schedulerModel->emptySignal.enter();
    running = true;
    schedulerModel->emptySignal.signal();
    schedulerModel->emptySignal.leave();

    // Execute until running is set to false to signal termination of the framework
    while (true)
    {
        // Sleep until wake up from scheduler
        signaler.wait();

        // For task and event execution leave critical area to scheduler
        signaler.leave();

        // When an event is pending, perform them.
        if (schedulerImpl->clock.isPending())
        {
            schedulerImpl->handleEvents();
        }
        for (TaskImpl* task = schedulerImpl->policy.nextTask(); (task != nullptr);
             task = schedulerImpl->policy.nextTask())
        {
            // When an event is pending, perform them first.
            if (schedulerImpl->clock.isPending())
            {
                schedulerImpl->handleEvents();
            }
            // Execute task
            schedulerImpl->execute(*task);
        }
        // Enter into critical section to have synchronization on running flag and signaler wait.
        signaler.enter();
        schedulerModel->emptySignal.enter();
        nextFree = schedulerModel->freeExecutors;
        schedulerModel->freeExecutors = this;
        schedulerModel->emptySignal.signal();
        schedulerModel->emptySignal.leave();
    } // end of execution loop

    // Leave critical section
    signaler.leave();
}

// ================

Tasking::SchedulerExecutionModel::SchedulerExecutionModel(SchedulePolicy& schedulePolicy, Executor* p_executors,
                                                          unsigned int executorNumber) :
    Scheduler(schedulePolicy, clockExecutionModel), //
    clockExecutionModel(*this),
    executors(p_executors),
    numberOfExecutors(executorNumber),
    freeExecutors(nullptr)
{
}

// ----------------

void
Tasking::SchedulerExecutionModel::startExecutors(void)
{
    // start all threads if this is the first start
    emptySignal.enter();
    for (unsigned int i = 0; i < numberOfExecutors; i++)
    {
        // Start only if it is not started yet. The threads did not terminate.
        if (!executors[i].running)
        {
            executors[i].startExecutor(*this);
            // Wait on signal from started thread to prevent running conditions
            emptySignal.wait();

            // Put executor to list of free executors
            executors[i].nextFree = freeExecutors;
            freeExecutors = executors + i;
        }
    }
    emptySignal.leave();
}

// ----------------

void
Tasking::SchedulerExecutionModel::signal(void)
{
    // Protect access to list of free executors
    emptySignal.enter();
    Executor* executor = freeExecutors;
    if (executor != nullptr)
    {
        // One executor is free, remove them from list of free executors and signal them for execution
        freeExecutors = executor->nextFree;
        // Protection of free executors is no longer needed.
        emptySignal.leave();
        executor->signaler.enter();
        executor->signaler.signal();
        executor->signaler.leave();
    }
    else
    {
        // No executor free, signaling is not necessary but leaving protected area
        emptySignal.leave();
    }
}

// ----------------

void
Tasking::SchedulerExecutionModel::waitUntilEmpty(void)
{
    // Go into critical section for empty signal
    emptySignal.enter();

    // Expect at first all are occupied
    unsigned int numberOfOccupiedExecutors = numberOfExecutors;

    // Tasks in the free list are not occupied, so no wait for them.
    for (Executor* executor = freeExecutors; executor != nullptr; executor = executor->nextFree)
    {
        --numberOfOccupiedExecutors;
    }
    // Remaining executors are occupied, wait on a signal from them until all signal free.
    while (numberOfOccupiedExecutors > 0u)
    {
        emptySignal.wait();
        --numberOfOccupiedExecutors;
    }
    // All events in the clock queue and tasks in the run queue are performed now
    emptySignal.leave();
}

// ----------------

void
Tasking::SchedulerExecutionModel::setZeroTime(Time offset)
{
    clockExecutionModel.setZeroTime(offset);
}
