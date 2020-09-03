/*
 * clockExecutionModel.h
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

#ifndef TASKING_ARCH_LINUX_CLOCKEXECUTIONMODEL_H_
#define TASKING_ARCH_LINUX_CLOCKEXECUTIONMODEL_H_

#include <pthread.h>
#include <impl/clock_impl.h>

namespace Tasking
{

extern "C"
{
    /// The main thread which handles clock activities of the scheduler instance.
    void* clockThread(void*);
}

/// Implementation of a clock execution model with the POSIX API.
class ClockExecutionModel : public Clock
{
    friend void* clockThread(void*);

public:
    /**
     * Initialize clock, create and start the pthread to manage the clock.
     * @param scheduler Reference to the executor
     */
    ClockExecutionModel(Scheduler& scheduler);

    /// Free memory of pthread stuff and terminate pthread thread
    ~ClockExecutionModel(void);

    /// @return Compute the Tasking time requested from the POSIX real time clock since start.
    Time getTime(void) const override;

    /**
     * Method to set the zero time.
     *
     * @param offset Offset time to which the zero time is adjusted. An immediate subsequent call to get
     * time will than deliver the value of this parameter.
     */
    void setZeroTime(Time offset);

protected:
    /**
     * Compute the current time added by a time span inside the timespec structure from the POSIX API.
     * The result will write to the wakeUpTime.
     * @param timeSpan Time span added to current time
     * @see wakeUpTime
     */
    void computeAbsoluteWakeUpTime(Time timeSpan);

    /**
     * Start the timer for a new wake up time.
     * @param timeSpan The timer after which the trigger shall start.
     */
    void startTimer(Time timeSpan) override;

    /// POSIX thread to handle the clock by timed wait calls
    pthread_t m_thread;

    /// POSIX mutex to synch start and termination of the clock thread and to implement timed wait.
    pthread_mutex_t m_mutex;

    /// POSIX conditional variable to implement timed wait on basis of the time out of conditional variables
    pthread_cond_t m_cond;

    /**
     * Flag to control the run of the clock thread during start and termination of the scheduler.
     */
    bool running;

    /**
     * Time of the computer when the process is started. It's needed to get for all periodic events the
     * same time base.
     */
    struct timespec zeroTime;

    /// Absolute time point when the clock thread should wake up from sleeping
    struct timespec wakeUpTime;
};

} // namespace Tasking

#endif /* TASKING_ARCH_LINUX_CLOCKEXECUTIONMODEL_H_ */
