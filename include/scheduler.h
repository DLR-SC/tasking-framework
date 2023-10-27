/*
 * scheduler.h
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

#ifndef TASKING_INCLUDE_SCHEDULER_H_
#define TASKING_INCLUDE_SCHEDULER_H_

#include "impl/scheduler_impl.h"

namespace Tasking
{

// Forward name declarations
struct TaskImpl;
struct TaskingAccessor;

/**
 * Common interface to the scheduler used by the Tasking Framework elements. It is recommended to use the template
 * class SchedulerProvider to instantiate a scheduler.
 * @see SchedulerProvider
 */
class Scheduler
{
    friend TaskingAccessor;

public:
    /**
     * Initialize the scheduler.
     *
     * @param schedulePolicy Reference to the used scheduling policy for the scheduler.
     * @param clock Reference to the clock used by the scheduler implementation
     */
    Scheduler(SchedulePolicy& schedulePolicy, Clock& clock);

    /// Virtual destructor of interface
    virtual ~Scheduler(void);

    /**
     * Set a zero time with an offset time to the current time when the function is called. By default a zero time
     * is set at construction time of the scheduler without offset, but for synchronization issues the clock can
     * adjusted to an outer signal from time to time.
     *
     * If the system is currently running, adjusting the clock will have an effect on the start time of all events,
     * because all time points to start an event in the clock queue are organized by absolute time points.
     *
     * The bare metal implementation has to implement this functionality.
     *
     * @param offset Offset time to the current time. Using the current time of the clock will have nearly no effect
     * to the timing.
     */
    virtual void setZeroTime(Time offset) = 0;

    /**
     * Start the scheduling of tasks.
     *
     * @param doReset If set to true, a reset on all associated tasks is performed. If set to false, each activated task
     * will be queued for execution.
     *
     * @see terminate
     */
    void start(bool doReset = true);

    /**
     * Stopping the scheduling of tasks. The scheduler didn't accept tasks to perform until start is called.
     *
     * @param doNotRemovePendingTasks If the flag is set to false, after stop acceptance of task activations is stopped,
     * pending tasks in the run queue are removed. Currently running tasks will not terminated by this call.
     *
     * @see start
     */
    void terminate(bool doNotRemovePendingTasks = false);

    /**
     * Call initialize method of all associated tasks of the scheduler. A task is associated to a task when it
     * is constructed with a reference to the scheduler instance.
     */
    void initialize(void);

    /**
     * Get the absolute time used to control events. The zero time depends on the bare metal implementation. Application
     * programmer can use this time for time stamps or to calculate the offset time of a periodic event.
     *
     * @result Time which is in the time frame used for triggering events in ms. Most of the time, zero time is start
     * of the system.
     *
     * @see Event::setPeriodicTiming
     * @see setZeroTime
     */
    Time getTime(void) const;

protected:
    /**
     * Pure abstract method which must be implemented by the bare metal implementation of the scheduler.
     * The method implementation shall wake up one of the executors of the scheduler instance. The method is called
     * whenever a new task should perform and the run queue is empty or an event is fired by the clock.
     */
    virtual void signal(void) = 0;

    /**
     * A call to the method waits until the run queue of the scheduler runs empty. If pending tasks activate other tasks
     * also this task will be executed before waitUntilEmpty returns. The bare metal model has to implement these
     * functionality to enable a safe termination of the Tasking Framework.
     */
    virtual void waitUntilEmpty(void) = 0;

    /**
     * @return Reference to the implementation part of the scheduler.
     */
    SchedulerImpl& getImpl(void);

private:
    SchedulerImpl impl;
};

} // namespace Tasking

// ---------------- inlines -----------------

inline Tasking::Time
Tasking::Scheduler::getTime() const
{
    return impl.clock.getTime();
}

inline Tasking::SchedulerImpl&
Tasking::Scheduler::getImpl(void)
{
    return impl;
}

#endif /* TASKING_INCLUDE_SCHEDULER_H_ */
