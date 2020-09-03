/*
 * schedulerExecutionModel.h
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

#ifndef TASKING_ARCH_LINUX_SCHEDULEREXECUTIONMODEL_H_
#define TASKING_ARCH_LINUX_SCHEDULEREXECUTIONMODEL_H_

#include <scheduler.h>
#include "signaler.h"
#include "clockExecutionModel.h"

namespace Tasking
{

/// Implementation of the scheduler execution model with means of the POSIX pthread library.
class SchedulerExecutionModel : public Scheduler
{
    friend void* executorThread(void*);
    friend void* clockThread(void*);

public:
    // Encapsulation of a POSIX thread as executor for the Tasking framework.
    struct Executor
    {
        /// Zero initialization of the Executor class.
        Executor(void);

        /// Terminate and free a POSIX thread.
        ~Executor(void);

        /**
         * Perform last initialization steps and allocate and start the POSIX thread as executor.
         * @param scheduler
         */
        void startExecutor(SchedulerExecutionModel& scheduler);

        /// Data field for thread information needed by the pthread library.
        pthread_t thread;

        /// Conditional semaphore to synchronize and wake up the executor thread by the scheduler.
        Signaler signaler;

        /// Pointer to the associated scheduler
        SchedulerExecutionModel* schedulerModel;

        /// Point to the associated implementation of the scheduler.
        SchedulerImpl* schedulerImpl;

        /// Flag to indicate the thread is running. Setting to false will terminate the thread.
        bool running;

        /// Flag to indicate the sleeping is wait on a signal from the scheduler. Needed to search for a free executor.
        bool waitOnSignal;

        /**
         * Pointer to the next free executor. The pointer is updated whenever the executor goes into wait state of
         * the signaler.
         */
        Executor* nextFree;
    };

    /**
     * Initialize execution model.
     * @param schedulePolicy The policy which is used by the scheduler.
     * @param executors Pointer to the array of executors which can used by the implementation.
     * @param numberOfExecutors Number of available executors in the array of executors.
     */
    SchedulerExecutionModel(SchedulePolicy& schedulePolicy, Executor* executors, unsigned int numberOfExecutors);

    /**
     * Set a zero time with an offset time to the current time when the function is called. By default a zero time
     * is set at construction time of the scheduler without offset, but for synchronization issues the clock can
     * be adjusted to an outer signal.
     *
     * If the system is currently running, adjusting the clock will have an effect on the start time of all events,
     * because all times in the run queue are absolute times.
     *
     * @param offset Offset time to the current time. Using the current time of the clock will have nearly no effect
     * to the timing.
     */
    void setZeroTime(Time offset) override;

protected:
    /** Start the executors. SchedulerExecutionModel is base class of provider and executors are child of provider.
     * Can not started earlier.
     */
    void startExecutors(void);

    /**
     * Search for an empty executor thread and wake them up. If no free executor thread is found do nothing.
     */
    void signal(void) override;

    /**
     * The method counts the number of executors processing currently a task or events and waits until all of them
     * are terminating its processing and going to wait. There should be no executors start processing of task
     * concurrent to the wait.
     */
    void waitUntilEmpty(void) override;

    /// The used clock execution model.
    ClockExecutionModel clockExecutionModel;

    /// Pointer to the executors.
    Executor* executors;

    /// Number of executors used by the execution model.
    unsigned int numberOfExecutors;

    /// A signaler to implement the wait until empty.
    Signaler emptySignal;

    /// Index to the first free executor or -1 if all occupied.
    Executor* freeExecutors;
};

} // namespace Tasking

#endif /* TASKING_ARCH_LINUX_SCHEDULEREXECUTIONMODEL_H_ */
