/*
 * taskPeriodicSchedule_impl.h
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

#ifndef INCLUDE_IMPL_TASKPERIODICSCHEDULE_IMPL_H_
#define INCLUDE_IMPL_TASKPERIODICSCHEDULE_IMPL_H_

#include "../taskTypes.h"

namespace Tasking {

class PeriodicScheduleTrigger;

/**
 * Implementation structure of an periodic schedule. This structure hold all data and
 * internal functions to process an periodic schedule.
 */
struct PeriodicScheduleImpl {

    /// Null initialization of periodic schedule
    PeriodicScheduleImpl(void);

    /**
     * Sort in a trigger into the list of triggers.
     *
     * @see triggers
     */
    void sortIn(PeriodicScheduleTrigger& trigger);

    /**
     * Loop over the next periodic triggers with the same offset and push them. As side effect active trigger move on.
     */
    void pushTriggers(void);

    /**
     * Step the activeTrigger to the next one with a different time or start from the beginning.
     *
     * @result The absolute time for the next trigger.
     */
    Tasking::Time stepToNextTriggerOffset(void);

    /// Sorted list of time trigger. Sorting order is from smalled
    PeriodicScheduleTrigger* triggers;

    /// Last activated trigger
    PeriodicScheduleTrigger* activeTrigger;

    /// Current start time of period
    Tasking::Time startTimeOffPeriod_ms;

    /// Period of schedule
    Tasking::Time period_ms;
};

} // namespace Tasking


#endif /* INCLUDE_IMPL_TASKPERIODICSCHEDULE_IMPL_H_ */
