/*
 * taskBarrier.h
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

/*
 * Define a barrier which activate the associated inputs when a specified number of push operations
 * is reached.
 */

#ifndef TASKING_TASK_BARRIER_H_
#define TASKING_TASK_BARRIER_H_

#include "taskChannel.h"
#include <mutex.h>

namespace Tasking
{

/**
 * The number of activations at an input is declared at compile time. In situations, where the number of
 * data elements is only known at run time, the activation cannot be adapted. This can be the case when for
 * example a data source has states where no data is sent.  The barrier is a mean to control the activation
 * of tasks with an unknown number of data packages.
 *
 * By default the barrier can be instantiated with a minimum number of expected push operations on the barrier.
 * After the minimum number of pushes has happened the barrier will activate all associated inputs, as long as data
 * sources did not increase the number of expected push operations on the channel. If its increased, a larger number
 * of push operations is expected.
 */
class Barrier : public Tasking::Channel
{
public:
    /**
     * Initialization of counter trigger.
     *
     * @param resetValue Value which is set at a reset of the barrier as minimum number of expected push operations.
     */
    Barrier(unsigned int resetValue = 0);

    /**
     * Increase the number of expected push operations until the counter trigger is activated.
     *
     * @param delta Delta value for the number of expected push operations. Default value is 1.
     */
    void increase(const unsigned int delta = 1);

    /**
     * Decrease the counter of expected push operations by one. If the counter falls back to 0, all associated
     * inputs are activated.
     */
    void push(void);

protected:
    /// Set the reset value for the next execution cycle.
    void reset(void) override;

private:
    /// Counter to count the number of push operations. If counter is zero the channel will be activated.
    unsigned int counter;

    /// Start value set during a reset
    unsigned int startValue;

    /// Mutex to protect concurrent access to the barrier.
    Mutex protection;
};

} // namespace Tasking

#endif /* TASKING_TASK_BARRIER_H_ */
