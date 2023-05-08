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

#include <cassert>
#include "schedulerExecutionModel.h"

namespace Tasking
{
extern "C"
{
    void*
    clockThread(void* clockExecutionModel)
    {
        ClockExecutionModel* clock = static_cast<ClockExecutionModel*>(clockExecutionModel);

        // For the wait calls a lock on the conditional mutex is necessary.
        pthread_mutex_lock(&(clock->m_mutex));
        clock->running = true;

        // Start main loop of thread. Thread will stop if the values of the time spec are both 0.
        while (clock->running)
        {

            // Waiting for signal from clock
            pthread_cond_wait(&clock->m_cond, &clock->m_mutex);

            // Keep in this loop so long a next wake-up time exists
            while (clock->running && !clock->isEmtpy())
            {
                // Going to wait until wake up time reached or a signal from the clock comes
                pthread_cond_timedwait(&(clock->m_cond), &(clock->m_mutex), &(clock->wakeUpTime));
                // There are several reasons for the wake-up.
                // Do only some actions when an event is pending, else start next clock thread cycle in inner or outer
                // loop
                if (clock->isPending())
                {
                    // Signal the scheduler, it will perform pending events
                    static_cast<SchedulerExecutionModel*>(&(clock->scheduler))->signal();
                    // Adjust wake-up time for next sleep
                    Tasking::Time timeSpan = clock->getNextStartTime() - clock->getTime();
                    clock->computeAbsoluteWakeUpTime(timeSpan);
                }
            } // end of loop over not empty clock event list
        } // end of loop over waiting on empty clock list

        // Unlocking the mutex. It's needed never again.
        pthread_mutex_unlock(&(clock->m_mutex));

        // Terminate thread
        pthread_exit(nullptr);
        //    return NULL; // Dead code by pthread_exit, but gives warning
    }
} // extern "C"
} // namespace Tasking

// ----------------

Tasking::ClockExecutionModel::ClockExecutionModel(Scheduler& p_scheduler) : Clock(p_scheduler)
{
    // Request the zero time as basis to define periodical timer.
    clock_gettime(CLOCK_REALTIME, &zeroTime);

    // Setting time structure for next wake-up to zero. This will hold clock thread in outer running loop
    wakeUpTime.tv_sec = 0;
    wakeUpTime.tv_nsec = 0;
    running = false;

    // Set up mutex, conditional variable and start thread
    int state = pthread_mutex_init(&m_mutex, nullptr);
    state |= pthread_cond_init(&m_cond, nullptr);
    state |= pthread_create(&m_thread, nullptr, clockThread, this);
    assert(state == 0);
    // Check start of the thread to prevent running condition. It must be inside the conditional wait to continue
    pthread_mutex_lock(&m_mutex);
    while (!running)
    {
        pthread_mutex_unlock(&m_mutex);
        // Wait a microsecond if the thread isn't ready yet. This only can appear
        struct timespec sleeptime;
        sleeptime.tv_sec = 0;
        sleeptime.tv_nsec = 1000;
        nanosleep(&sleeptime, nullptr);
        pthread_mutex_lock(&m_mutex);
    }
    pthread_mutex_unlock(&m_mutex);
}

// ----------------

Tasking::ClockExecutionModel::~ClockExecutionModel(void)
{
    // Terminating thread
    running = false;
    // Wake up thread
    pthread_cond_signal(&m_cond);
    // Wait on termination of the thread
    pthread_join(m_thread, nullptr);
    pthread_detach(m_thread);
    // Destroy conditional variable and mutex
    pthread_cond_destroy(&m_cond);
    pthread_mutex_destroy(&m_mutex);
}

// ----------------

Tasking::Time
Tasking::ClockExecutionModel::getTime(void) const
{
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    // Return time as milliseconds
    return (now.tv_sec - zeroTime.tv_sec) * 1000 + (now.tv_nsec - zeroTime.tv_nsec) / 1000000;
}

// ----------------

void
Tasking::ClockExecutionModel::setZeroTime(Tasking::Time offset)
{
    // Request the zero time from the system
    struct timespec newZeroTime;
    clock_gettime(CLOCK_REALTIME, &newZeroTime);
    // Correct by offset time as new start value of the clock
    newZeroTime.tv_sec -= offset / 1000;
    newZeroTime.tv_nsec -= offset * 1000000;
    if (newZeroTime.tv_nsec < 0)
    {
        newZeroTime.tv_nsec += 1000000000;
        newZeroTime.tv_sec--;
    }
    // Copy to zero time
    zeroTime.tv_nsec = newZeroTime.tv_nsec;
    zeroTime.tv_sec = newZeroTime.tv_sec;
}

// ----------------

void
Tasking::ClockExecutionModel::computeAbsoluteWakeUpTime(Time timeSpan)
{
    clock_gettime(CLOCK_REALTIME, &wakeUpTime);
    wakeUpTime.tv_sec += timeSpan / 1000;
    wakeUpTime.tv_nsec += (timeSpan % 1000) * 1000000;
    if (wakeUpTime.tv_nsec > 1000000000)
    {
        wakeUpTime.tv_nsec -= 1000000000;
        wakeUpTime.tv_sec++;
    }
}

// ----------------

void
Tasking::ClockExecutionModel::startTimer(Time timeSpan)
{
    // Compute the absolute time to wake up
    pthread_mutex_lock(&m_mutex); // Wake-up time is also changed by clock thread
    computeAbsoluteWakeUpTime(timeSpan);
    pthread_cond_signal(&m_cond);
    pthread_mutex_unlock(&m_mutex);
}
