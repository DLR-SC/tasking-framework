/*
 * taskStatistics.h
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

#ifndef TASKING_STATISTICS_H_
#define TASKING_STATISTICS_H_

#include "taskTypes.h"

namespace Tasking
{

class Scheduler;
class Clock;

/** Class to manage diagnostic data in the tasking framework.
 * In the current version it is not supported by one of the scheduler implementations.
 */
class TaskingStatistics
{
    friend class Scheduler;
    friend class Clock;

public:
    /// Structure to hold the statisitical data.
    struct Statistic
    {
        unsigned int lostActivations;
        unsigned int lostEvents;
        unsigned int maxRunQueueLength;
        unsigned int maxEvents;
        Time maxQueingTime;
    };

    /// Initialize data
    TaskingStatistics(void);

    /// Set all statistic data to zero except current values
    void clear(void);

    /**
     * Read out the current statistical data. Read out will set all values back to zero or the current state
     * @param currentStatistic [out] Reference to the statistic structure to fill in with the current statistics.
     */
    void read(Statistic& currentStatistic);

private:
    void reportActivation(void);
    void reportLostActivation(void);
    void reportTermination(void);
    void reportIdle(void);
    void reportAddEvent(void);
    void reportFireEvent(void);
    void reportLostEvent(void);
    void reportQueuingTime(Time queueingTime);

    /// Memory area to hold the statistics
    volatile Statistic states;

    unsigned int currentRunQueueLength;
    unsigned int currentEventNumber;
};

extern TaskingStatistics statistics;

inline void
TaskingStatistics::reportActivation(void)
{
    ++currentRunQueueLength;
    if (currentRunQueueLength > states.maxRunQueueLength)
    {
        states.maxRunQueueLength = currentRunQueueLength;
    }
}
inline void
TaskingStatistics::reportLostActivation(void)
{
    ++states.lostActivations;
}
inline void
TaskingStatistics::reportTermination(void)
{
    --currentRunQueueLength;
}
inline void
TaskingStatistics::reportIdle(void)
{
}
inline void
TaskingStatistics::reportAddEvent(void)
{
    ++currentEventNumber;
    if (currentEventNumber > states.maxEvents)
    {
        states.maxEvents = currentEventNumber;
    }
}
inline void
TaskingStatistics::reportFireEvent(void)
{
    --currentEventNumber;
}
inline void
TaskingStatistics::reportLostEvent(void)
{
    ++states.lostEvents;
}
inline void
Tasking::TaskingStatistics::reportQueuingTime(Time queueingTime)
{
    if (queueingTime > states.maxQueingTime)
    {
        states.maxQueingTime = queueingTime;
    }
}
} // namespace Tasking

#endif /* EXT_TASKING_INCLUDE_TASKSTATISTICS_H_ */
