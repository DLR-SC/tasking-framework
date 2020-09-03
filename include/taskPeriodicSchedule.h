/*
 * taskPeriodicSchedule.h
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

#ifndef INCLUDE_TASKPERIODICSCHEDULE_H_
#define INCLUDE_TASKPERIODICSCHEDULE_H_

#include "impl/taskPeriodicSchedule_impl.h"
#include "taskChannel.h"

namespace Tasking
{

class EventImpl;
// Forward definition of the event

/**
 * Class to define a time trigger in the periodic schedule. This class acts as a channel and will be connected to
 * a task input by the application software.
 *
 * @see Task::configureInput
 */
class PeriodicScheduleTrigger : public Channel
{
    friend PeriodicScheduleImpl;

public:
    /**
     * Define the time trigger in the periodic series.
     * @param offset Offset time inside the period of the time schedule. If the offset is outside of the periodic
     * schedule period, the time trigger will never fired.
     */
    explicit PeriodicScheduleTrigger(Time offset);

private:
    /// Offset time inside the period
    Time offsetTime;

    /// Pointer to the next time trigger in the order smallest to highest offset time
    PeriodicScheduleTrigger* next;
};
// class Trigger

/**
 * A periodic schedule is an series of time triggers, which will be triggered in each period. The class is used
 * to define a fix time schedule. The periodic schedule can be connected to an event. If this event is scheduled as
 * periodic event, the event will fire at the trigger times of the periodic schedule.
 *
 * The class hold a sorted list of time triggers. These triggers act as channels and are connected to the inputs
 * of the task to start by the time trigger.
 *
 * @see Event
 */
class PeriodicSchedule
{
    friend EventImpl;

public:
    /**
     * Add a time trigger to the schedule.
     *
     * @param trigger Reference to the trigger added to the schedule. The offset of the trigger should inside of the
     * period of the periodic schedule, else the trigger will not scheduled.
     */
    void add(PeriodicScheduleTrigger& trigger);

private:
    PeriodicScheduleImpl impl;
};

} // namespace Tasking

#endif /* INCLUDE_TASKPERIODICSCHEDULE_H_ */
