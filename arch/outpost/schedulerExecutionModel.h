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

#ifndef TASKING_ARCH_OUTPOST_SCHEDULEREXECUTIONMODEL_H_
#define TASKING_ARCH_OUTPOST_SCHEDULEREXECUTIONMODEL_H_

#include <scheduler.h>
#include "signaler.h"
#include "clockExecutionModel.h"

namespace Tasking
{

/// Implementation of the scheduler execution model with the means of the Outpost library.
class SchedulerExecutionModel : public Scheduler
{
    friend class ClockExecutionModel;

public:
    // Outpost thread implementation to execute tasks and handle events of the Tasking Framework.
    class Executor : public outpost::rtos::Thread
    {
    public:
        /// Setup executor as outpost thread.
        Executor(void);

        /// Only to prevent compiler warnings.
        virtual ~Executor(void);

        /**
         * Perform last initialization steps and start the executor thread of the executor.
         * @param scheduler
         */
        void startExecutor(SchedulerExecutionModel& scheduler);

        /// Thread body to execute tasks and handle events.
        virtual void run(void);

        /// Signaler for the wake up mechanism.
        Signaler signaler;

        /// Flag to indicate that the thread is started yet.
        bool running;

        /// Point to the associated scheduler
        SchedulerExecutionModel* schedulerModel;

        /// Point to the associated implementation of the scheduler.
        SchedulerImpl* schedulerImpl;

        /// Pointer to the next free executor or a null pointer. The pointer is updated when the executor gets free.
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
     * Setting the zero time in the system. By default the zero time is set when the scheduler is constructed and
     * can be adjusted later for synchronization purposes.
     * @param offset Time point with the exact time when the method is called. An immediate request of the time will
     * be equal to the offset.
     */
    void setZeroTime(Time offset) override;

protected:
    /**
     * Starting all executors. The method is called by the provider when all executors are initialized to keep the
     * correct ordering at start of all threads in the scheduler.
     */
    void startExecutors(void);

    /**
     * Implementation to search for a free executor and trigger them to execute pending tasks or handle pending events.
     */
    void signal(void) override;

    /**
     * Wait until all executors finalize all running tasks and events. When the method returns, all executors will be
     * in the wait state of its signaler.
     */
    void waitUntilEmpty(void) override;

    /// The used clock execution model with Outpost means.
    ClockExecutionModel clockExecutionModel;

    /// Pointer to all executors.
    Executor* executors;

    /// Number of executors used by the execution model.
    unsigned int numberOfExecutors;

    /// A signaler to establish a synchronization between executors and the scheduler.
    Signaler emptySignal;

    /// Pointer the list of all executors in the wait state of its signaler.
    Executor* freeExecutors;
};

} // namespace Tasking

#endif /* TASKING_ARCH_OUTPOST_SCHEDULEREXECUTIONMODEL_H_ */
