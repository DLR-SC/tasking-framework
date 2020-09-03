/*
 * signaler.h
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

#ifndef TASKING_INCLUDE_ARCH_OUTPOST_SIGNALER_H_
#define TASKING_INCLUDE_ARCH_OUTPOST_SIGNALER_H_

#include <outpost/rtos/semaphore.h>
#include "mutex.h"

namespace Tasking
{

/// Implementation of a signaler on base of outpost mutex variables
class Signaler : public Mutex
{
public:
    /// Lock the blocking semaphore at instantiation
    Signaler(void);

    /**
     * Wait until another concurrent software component calls the signal method of these signaler. When the method
     * is left, the wake up flag will be false. The method should only called after the signaler is locked.
     * @see signal
     */
    void wait(void);

    /**
     * Send a signal to a waiting task to wake up the task. The operation should only called from
     * inside the monitor, else it is resulting in an unexpected behavior.
     * @see wait
     * @see Mutex::enter
     */
    void signal(void);

protected:
    /// Blocking Semaphore
    outpost::rtos::BinarySemaphore block;
    /// Number of wait calls which are not wake up yet
    unsigned int pendingWakeUps;
    /// Only one get the chance to wake up by one wake up call
    volatile bool wakeUp;
};

} // namespace Tasking

#endif /* TASKING_INCLUDE_ARCH_OUTPOST_SIGNALER_H_ */
