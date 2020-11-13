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

#include <cassert>

#include "schedulerExecutionModel.h"

// ===== Body of the executor thread =====

namespace Tasking
{

void*
executorThread(void* management)
{
    assert((management != 0));
    Tasking::SchedulerExecutionModel::Executor* data =
            static_cast<Tasking::SchedulerExecutionModel::Executor*>(management);

    // Go inside critical section for start up of thread
    data->signaler.enter();

    // Sign thread as started.
    data->running = true;

    // Execute until running is set to false to signal termination of the framework
    while (data->running)
    {
        // Sleep until wake up from scheduler
        data->waitOnSignal = true;
        data->signaler.wait();
        data->waitOnSignal = false;

        // For task and event execution leave critical area to scheduler
        data->signaler.leave();

        // When an event is pending, perform them.
        if (data->schedulerImpl->clock.isPending())
        {
            data->schedulerImpl->handleEvents();
        }
        for (Tasking::TaskImpl* task = data->schedulerImpl->policy.nextTask(); (task != nullptr) && data->running;
             task = data->schedulerImpl->policy.nextTask())
        {
            // Execute task
            data->schedulerImpl->execute(*task);
            // Maybe after execution of task some new events are pending
            if (data->schedulerImpl->clock.isPending())
            {
                data->schedulerImpl->handleEvents();
            }
        }
        // Enter into critical section to have synchronization on running flag and signaler wait.
        data->signaler.enter();
        data->schedulerModel->emptySignal.enter();
        data->nextFree = data->schedulerModel->freeExecutors;
        data->schedulerModel->freeExecutors = data;
        data->schedulerModel->emptySignal.signal();
        data->schedulerModel->emptySignal.leave();
    } // end of execution loop

    // Leave critical section
    data->signaler.leave();

    // Terminate pthread and deliver a null pointer as result.
    pthread_exit(nullptr);
    return nullptr;
}
} // namespace Tasking

// ================

Tasking::SchedulerExecutionModel::Executor::Executor(void) :
    thread(0u), schedulerModel(nullptr), schedulerImpl(nullptr), running(false), waitOnSignal(false), nextFree(nullptr)
{
}

// ----------------

Tasking::SchedulerExecutionModel::Executor::~Executor()
{
    // Enter into critical area of the executor thread
    signaler.enter();
    // Set running variable to termination of endless loop
    running = false;
    // ... and signal the executor to wake up if it is in the wait.
    // If not in the wait, the task is executing tasks currently and will not go into sleep by the running flag.
    if (waitOnSignal)
    {
        signaler.signal();
    }
    signaler.leave();
    int success = pthread_join(thread, nullptr);
    assert((success == 0));
    pthread_detach(thread);
}

// ----------------

void
Tasking::SchedulerExecutionModel::Executor::startExecutor(SchedulerExecutionModel& p_scheduler)
{
    schedulerModel = &p_scheduler;
    schedulerImpl = &p_scheduler.getImpl();

    // Set up and starting thread
    int success = pthread_create(&thread, nullptr, executorThread, this);
    assert((success == 0));

    // We can get here a race condition if the thread is not started before the new created thread
    // is in the synchMutex. So we have to check this first and delay if this is not the case. If
    // running is set, the thread is in the conditional wait and can wake up by a signal.
    signaler.enter();
    while (!running)
    {
        signaler.leave();
        // and wait a millisecond if the thread isn't ready yet.
        // This  can appear by the latency of the POSIX scheduler at thread start
        struct timespec sleeptime;
        sleeptime.tv_sec = 0;
        sleeptime.tv_nsec = 1000;
        nanosleep(&sleeptime, nullptr);
        signaler.enter();
    }
    signaler.leave();
}

// ================

Tasking::SchedulerExecutionModel::SchedulerExecutionModel(SchedulePolicy& schedulePolicy, Executor* _executors,
                                                          unsigned int executorNumber) :
    Scheduler(schedulePolicy, clockExecutionModel), //
    clockExecutionModel(*this),
    executors(_executors),
    numberOfExecutors(executorNumber),
    freeExecutors(nullptr)
{
}

// ----------------

void
Tasking::SchedulerExecutionModel::startExecutors(void)
{
    // Start the executor threads
    for (unsigned int i = 0; i < numberOfExecutors; ++i)
    {
        executors[i].startExecutor(*this);
        // Hang in free list after start
        executors[i].nextFree = freeExecutors;
        freeExecutors = executors + i;
    }
}

// ----------------

void
Tasking::SchedulerExecutionModel::setZeroTime(Time offset)
{
    clockExecutionModel.setZeroTime(offset);
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
