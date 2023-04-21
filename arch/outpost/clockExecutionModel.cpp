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

#include "clockExecutionModel.h"
#include "schedulerExecutionModel.h"

Tasking::ClockExecutionModel::ClockExecutionModel(Scheduler& p_scheduler) :
    Clock(p_scheduler), m_timer(this, &ClockExecutionModel::clockTick, "TSKC")
{
    m_zeroTime = boardClock.now();
}

// ----------------

Tasking::ClockExecutionModel::~ClockExecutionModel(void)
{
    // Stop the timer
    m_timer.cancel(); // Without the case the type could not resolved
}

// ----------------

Tasking::Time
Tasking::ClockExecutionModel::getTime(void) const
{
    return (boardClock.now() - m_zeroTime).milliseconds();
}

// ----------------

void
Tasking::ClockExecutionModel::setZeroTime(Tasking::Time p_offset)
{
    outpost::time::Milliseconds offset(p_offset);
    m_zeroTime = boardClock.now() - offset;
}

// ----------------

void
Tasking::ClockExecutionModel::startTimer(Time timeSpan)
{
    outpost::time::Milliseconds wakeUpIn(timeSpan);
    m_timer.start(wakeUpIn);
}

// ----------------

void
Tasking::ClockExecutionModel::clockTick(outpost::rtos::Timer* timer)
{
    // Input variable timer should equal to m_timer, but interface need it in that way.
    // Is a event is pending, signal
    if (isPending())
    {
        static_cast<SchedulerExecutionModel&>(scheduler).signal();
        Time nextGapTime = getNextStartTime() - getTime();
        if (0u != nextGapTime)
        {
            outpost::time::Milliseconds wakeUpIn(nextGapTime);
            timer->start(wakeUpIn);
        }
    }
    else
    {
        if (!isEmtpy())
        {
            outpost::time::SpacecraftElapsedTime nextStartTime =
                    m_zeroTime + outpost::time::Milliseconds(getHeadTime());
            timer->start(nextStartTime - boardClock.now());
        }
    }
}
