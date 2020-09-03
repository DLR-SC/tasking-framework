/*
 * taskEvent.h
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

#ifndef TASKEVENT_H_
#define TASKEVENT_H_

#include "impl/taskEvent_impl.h"

namespace Tasking
{

// Forward definition of periodic schedule
class PeriodicSchedule;

/**
 * The task event is a timed event. The behavior of the event can be periodically or relative to the
 * call of the method reset.
 *
 * The implementation specializes the class Channel with timing functionalities. An application programmer can
 * specialize the task event by overriding the two methods shallFire and onFire with own functionalities.
 *
 * @see TaskChannel
 */
class Event : public Channel
{
public:
    /**
     * @param scheduler Reference to the scheduler responsible to execute the event.
     * @param eventId Identifier for this channel.
     *
     * NOTE:
     *    It is the responsibility of the user to ensure uniqueness of the channel and events identifications.
     */
    explicit Event(Scheduler& scheduler, ChannelId eventId = 0);

    /**
     * @param scheduler Reference to the scheduler responsible to execute the event.
     * @param eventName Null-terminated string specifying a name for this event. The name will be
     *                  truncated after 4 characters.
     */
    explicit Event(Scheduler& scheduler, const char* eventName);

    /*
     * Destructor of the task event
     */
    ~Event(void);

    /**
     * Set the timing of event to a fix periodic behavior. Call this method only: from a constructor, when
     * the scheduler is initializing, or when the timer is stopped.
     *
     * @param period Period time in case of a periodical clock. A period of zero will lead to a single shot with
     * an absolute time
     *
     * @param offset Offset of the start time of the system. If the offset is in the past, the method computes
     * the next time point in the future by adding a multiple of the period to the offset. For a single shot with
     * period zero this event is fired immediately.
     */
    void setPeriodicTiming(const Time period, const Time offset);

    /**
     * Set the timing of event to play schedule of periodic triggers. Call this method only: from a constructor,
     * when the scheduler is initializing, or when the timer is stopped.
     * In this configuration this event itself will not notify an associated task input, only the periodic triggers in
     * the periodic schedule notifies associated task inputs. To change this behavior, the method shallFire can be
     * overridden.
     *
     * @param period Period time in case of a periodical clock. If the trigger time of the first periodic trigger in
     * the periodic schedule is not within the given period, the event is not started to play the periodic schedule.
     *
     * @param offset Offset of the start time of the system. If the offset is in the past, the method computes
     * the next time point in the future by adding a multiple of the period to the offset.
     *
     * @param schedule Reference to the schedule of periodic triggers to play by the event. If triggers are in the
     * schedule with an bigger offset than the period of the event, these triggers will not fired.
     *
     * @see shallFire
     */
    void setPeriodicSchedule(const Time period, const Time offset, PeriodicSchedule& schedule);

    /**
     * Set the timing of the event relative to the reset operation. A call to reset will trigger the task event
     * for the next activation. To start the relative timing a call to the reset operation is necessary. Keep in
     * mind that a reset restarts the timer, when the event is connected to several tasks or a final input is
     * connected to the task.
     *
     * @param delay Delay time in milliseconds which is used as trigger time relative to the reset operation.
     */
    void setRelativeTiming(const Time delay);

    /**
     * Trigger the event out of order. When the event is configured to periodic or relative timing the call of
     * the method has no effect, until the periodic or relative timing is stopped. An event can be only triggered
     * once. If it is queued by the clock, the event is removed from the clock before it is queued again. This
     * means reset operations on connected tasks will stop the event timer, e.g. when the event is connected to
     * several tasks or anconnected task with an input configured as final.
     *
     * @param time Offset time in ms when the event is triggered out of order. This can use to trigger an
     * task after a specified time to another task.
     *
     * @see setPeriodicTiming
     * @see setRelativTiming
     */
    void trigger(Time time = 0);

    /// @return True, when the clock is still queued for triggering at the clock.
    bool isTriggered(void) const;

    /**
     * Remove the task event from the list of time events in the clock. The event will not fire until a new
     * timing is programmed to the task event.
     */
    void stop(void);

    /**
     * Reset the task event. In case of a relative timing this method starts the timer and calls the
     * reset method of the overridden channel.
     */
    void reset(void) override;

    /**
     * The method is called when the event is handled.
     * @result By default true, so long no periodic schedule is played by the event. If the method or an override
     * return false, the associated input is not notified.
     */
    virtual bool shallFire(void);

    /**
     * The method is called every time the task event is handled by the schedule. The method can be overridden by
     * by the application software. By default it does nothing.
     */
    virtual void onFire(void);

    /**
     * @result Current time of the associated scheduler.
     */
    Tasking::Time now(void) const;

private:
    /// Structure for implementation
    EventImpl impl;
};

} // namespace Tasking

#endif /* TASKEVENT_H_ */
