/*
 * schedulerProvider.h
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

#ifndef TASKING_INCLUDE_SCHEDULERPROVIDER_H_
#define TASKING_INCLUDE_SCHEDULERPROVIDER_H_

#include <type_traits>

#include "schedulerExecutionModel.h"

namespace Tasking
{

/**
 * Template to instantiate a scheduler with all needed elements.
 * @tparam tp_numberOfExecutors Number of instantiated and started executors for the scheduler.
 * @tparam SchedulePolicyType Type name of the selected scheduling policy. All tasks associated to
 * this scheduler should follow the same scheduling policy.
 */
template<size_t tp_numberOfExecutors, typename SchedulePolicyType>
class SchedulerProvider : public SchedulerExecutionModel
{
public:
    /// Initialize the scheduler with the executors and the scheduling policy and start executors.
    SchedulerProvider(void);

protected:
    /// Instance of the policy to manage the run queue
    SchedulePolicyType policy;

    /// Pool of executors
    SchedulerExecutionModel::Executor executors[tp_numberOfExecutors];

private:
    using Tasking::Scheduler::getImpl;
    using Tasking::Scheduler::signal;
    using Tasking::Scheduler::waitUntilEmpty;
};

} // namespace Tasking

// ----------- inlines -----------

template<size_t tp_numberOfExecutors, typename SchedulePolicyType>
inline Tasking::SchedulerProvider<tp_numberOfExecutors, SchedulePolicyType>::SchedulerProvider(void) :
    SchedulerExecutionModel(policy, executors, tp_numberOfExecutors)
{
    static_assert(std::is_base_of<SchedulePolicy, SchedulePolicyType>::value,
                  "Schedule policy type shall be derived from Tasking::SchedulePolicy");

    // By limitations for array parameters and the order of the constructors
    // the executor can not start before without uninitialized data.
    startExecutors();
}

#endif /* TASKING_INCLUDE_SCHEDULERPROVIDER_H_ */
