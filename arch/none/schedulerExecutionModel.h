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

#ifndef TASKING_ARCH_NONE_SCHEDULEREXECUTIONMODEL_H_
#define TASKING_ARCH_NONE_SCHEDULEREXECUTIONMODEL_H_

#include <scheduler.h>
#include "clockExecutionModel.h"

namespace Tasking
{

/// Implementation of the scheduler execution model without any functionality
class SchedulerExecutionModel : public Scheduler
{
public:
    // In this execution model no executor exist. But class definition is needed
    class Executor
    {
    };

    /**
     * Initialize execution model
     * @param schedulePolicy The policy which is used by the scheduler.
     * @param executors Pointer to the array of executors which can used by the implementation
     * @param numberOfExecutors Number of available executors in the array of executors
     */
    SchedulerExecutionModel(SchedulePolicy& schedulePolicy, Executor* executors, unsigned int numberOfExecutors);

    /**
     * Setting the zero time in the system. By default the zero time is set when the clock is constructed and
     * can adjusted later for synchronization purposes. Not supported by none version
     * @param offset Not used by implementation
     */
    void setZeroTime(Time offset) override;

protected:
    /// Starting all executors. The method is called by the provider when the executors are initialized.
    void startExecutors(void);

    /**
     * Overloading of the pure abstract method which implement no functionality in this execution model
     */
    void signal(void) override;

    /**
     * Implementation of the pure virtual method without any functionality in this execution model
     */
    void waitUntilEmpty(void) override;

    /// The used clock execution model
    ClockExecutionModel clockExecutionModel;
};

} // namespace Tasking

#endif /* TASKING_ARCH_NONE_SCHEDULEREXECUTIONMODEL_H_ */
