/*
 * clockExecutionModel.cpp
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

#include "clockExecutionModel.h"

Tasking::ClockExecutionModel::ClockExecutionModel(Scheduler& _scheduler) :
    Clock(_scheduler), zeroTime(0u), currentTime(0u)
{
    // Nothing further to do for this implementation
}

// ----------------

void Tasking::ClockExecutionModel::startTimer(Time)
{
    std::clog << "void Tasking::ClockExecutionModel::startTimer" << std::endl;
}

// ----------------

void
Tasking::ClockExecutionModel::tick(void)
{
    ++currentTime;
    std::clog << "Tasking::ClockExecutionModel::tick: New time is " << currentTime << std::endl;
}

// ----------------

Tasking::Time
Tasking::ClockExecutionModel::getTime(void) const
{
    Time now = currentTime - zeroTime;
    std::clog << "Tasking::ClockExecutionModel::getTime: Returns " << now << std::endl;
    return now;
}

// ----------------

void
Tasking::ClockExecutionModel::setZeroTime(Time offset)
{
    zeroTime = offset;
    // Protect against underflow
    if (offset > currentTime)
    {
        currentTime = zeroTime;
    }
    std::clog << "Tasking::ClockExecutionModel::setZeroTime(" << offset << "): (zeroTime, currentTime) = (" << zeroTime
              << ", " << currentTime << ")" << std::endl;
}
