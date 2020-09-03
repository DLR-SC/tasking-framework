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

#include <iostream>

#include "schedulerExecutionModel.h"

Tasking::SchedulerExecutionModel::SchedulerExecutionModel(SchedulePolicy& schedulePolicy, Executor*, unsigned int) :
    Scheduler(schedulePolicy, clockExecutionModel), //
    clockExecutionModel(*this)
{
}

// ----------------

void
Tasking::SchedulerExecutionModel::startExecutors(void)
{
    std::clog << "Tasking::SchedulerExecutionModel::startExecutors" << std::endl;
}

// ----------------

void
Tasking::SchedulerExecutionModel::signal(void)
{
    std::clog << "Tasking::SchedulerExecutionModel::signal" << std::endl;
}

// ----------------

void
Tasking::SchedulerExecutionModel::waitUntilEmpty(void)
{
    std::clog << "Tasking::SchedulerExecutionModel::waitUntilEmpty" << std::endl;
    step();
}

// ----------------

void
Tasking::SchedulerExecutionModel::setZeroTime(Tasking::Time zeroTime)
{
    std::clog << "Tasking::SchedulerExecutionModel::setZeroTime(" << zeroTime << ")" << std::endl;
    clockExecutionModel.setZeroTime(zeroTime);
}

// ----------------

void
Tasking::SchedulerExecutionModel::step(void)
{
    std::clog << "Tasking::SchedulerExecutionModel::step" << std::endl;
    clockExecutionModel.tick();
    SchedulerImpl& schedulerImpl = Scheduler::getImpl();
    // Check for pending events and handle them
    if (schedulerImpl.clock.isPending())
    {
        schedulerImpl.handleEvents();
    }
    // Loop until all pending tasks are executed. This lead to an endless loop, when there is a circular activation
    // between tasks.
    for (Tasking::TaskImpl* taskToExecute = schedulerImpl.policy.nextTask(); nullptr != taskToExecute;
         taskToExecute = schedulerImpl.policy.nextTask())
    {
        schedulerImpl.execute(*taskToExecute);
        // Maybe the task lead as result to pending events, check and execute them.
        if (schedulerImpl.clock.isPending())
        {
            schedulerImpl.handleEvents();
        }
    }
}
