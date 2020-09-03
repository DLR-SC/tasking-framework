/*
 * schedulerUnitTest.h
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

#ifndef TASKING_INCLUDE_SCHEDULERUNITTEST_H_
#define TASKING_INCLUDE_SCHEDULERUNITTEST_H_

#include <scheduler.h>
#include <impl/clock_impl.h>

namespace Tasking
{

/**
 * Implementation of a scheduler for unit tests. This scheduler has no relation to a bare metal model and provides
 * no executors. The execution of pending tasks is performed by the unit test itself by calling the schedule method.
 */
class SchedulerUnitTest : public Scheduler
{
public:
    /**
     * Initialization for unit tests
     *
     * @param Reference to the used schedulePolicy.
     */
    SchedulerUnitTest(SchedulePolicy& schedulePolicy);

    /**
     * Execute all pending tasks.
     * @param timeSpan Time step in milliseconds the clock is forwarded in one step. If in the time span more than one
     * event is triggered, the unit test behaves not like an implemented system. Also an event with a higher frequency
     * than the time span will only trigger one time.
     */
    void schedule(Time timeSpan = 0u);

    /**
     * For this scheduler behavior is the same as calling schedule without a time span.
     */
    void waitUntilEmpty(void) override;

    /**
     * For unit tests adjusting the clock is not supported, because the clock is simulated.
     * A call of the method has no effect.
     *
     * @param offset Offset time to the current time.
     */
    void setZeroTime(Time offset) override;

protected:
    /**
     * Implementation of a simulated clock for unit tests.
     */
    class ClockUnitTest : public Clock
    {
    public:
        /**
         * Set up the unit test clock.
         * @param scheduler Reference to the unit test scheduler.
         */
        ClockUnitTest(SchedulerUnitTest& scheduler);

        /// @return Current simulated time in ms.
        Time getTime(void) const override;

        /**
         * Step forward in the simulated time.
         * @param timeSpan Time step in milliseconds the clock is forwarded.
         */
        void step(Tasking::Time span);

        /**
         * Needed by scheduler interface. For a simulated clock it is not needed. So, call has no effect.
         */
        void startTimer(Tasking::Time timeSpan) override;

    private:
        /// Simulated clock time.
        Time now;
    };

    /**
     * Needed by scheduler interface. For this scheduler type no signaling of executors are provided.
     */
    void signal(void) override;

    /// Instance of the used clock for the unit test.
    ClockUnitTest unitTestclock;
};
// class SchedulerUnitTest

} // namespace Tasking

#endif /* TASKING_INCLUDE_SCHEDULERUNITTEST_H_ */
