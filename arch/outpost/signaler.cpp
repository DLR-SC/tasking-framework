/*
 * signaler.cpp
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

#include "signaler.h"

Tasking::Signaler::Signaler(void) :
    block(outpost::rtos::BinarySemaphore::State::acquired), //
    pendingWakeUps(0), //
    wakeUp(false)
{
}

// ----------------

void
Tasking::Signaler::wait(void)
{
    pendingWakeUps++;
    do
    {
        mutex.release();
        // Going to sleep, because by default the semaphore is acquired.
        block.acquire();
        mutex.acquire();
    } while (!wakeUp); // Check that only one is wake up
    wakeUp = false;
    pendingWakeUps--;
}

// ----------------

void
Tasking::Signaler::signal(void)
{
    // Access to pendingWakeUps works, because signal and wait are only called from a caller which is inside the
    // monitor. Should someone wake up?
    if (pendingWakeUps > 0)
    {
        wakeUp = true;
        block.release();
    }
}
