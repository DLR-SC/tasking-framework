/*
 * schedulerUnitTest.cpp
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

#include <schedulerUnitTest.h>

Tasking::SchedulerUnitTest::SchedulerUnitTest(SchedulePolicy& schedulePolicy) :
    Scheduler(schedulePolicy, unitTestclock), unitTestclock(*this)
{
}

// ------------------------------------

void
Tasking::SchedulerUnitTest::signal(void)
{
    // No action to this scheduler type
}

// ------------------------------------

void
Tasking::SchedulerUnitTest::schedule(Tasking::Time timeSpan)
{
    // Step forward in time
    unitTestclock.step(timeSpan);
    // Doing execution
    SchedulerImpl& implementation = getImpl();
    do
    {
        implementation.handleEvents();
        for (TaskImpl* task = implementation.policy.nextTask(); (task != nullptr);
             task = implementation.policy.nextTask())
        {
            implementation.execute(*task);
        }
    } while (implementation.clock.isPending());
}

// ------------------------------------

void
Tasking::SchedulerUnitTest::waitUntilEmpty(void)
{
    schedule();
}

// ------------------------------------

void Tasking::SchedulerUnitTest::setZeroTime(Time)
{
}

// ====================================

Tasking::SchedulerUnitTest::ClockUnitTest::ClockUnitTest(SchedulerUnitTest& pSchedule) : Clock(pSchedule), now(0u)
{
}

// ------------------------------------

Tasking::Time
Tasking::SchedulerUnitTest::ClockUnitTest::getTime(void) const
{
    return now;
}

// ------------------------------------

void
Tasking::SchedulerUnitTest::ClockUnitTest::step(Tasking::Time span)
{
    now += span;
}

// ------------------------------------

void Tasking::SchedulerUnitTest::ClockUnitTest::startTimer(Tasking::Time)
{
    // No action to this scheduler type
}
