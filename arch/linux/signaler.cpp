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

#include <cassert>
#include "signaler.h"

Tasking::Signaler::Signaler(void) : wakeUp(false)
{
    int success;
    success = pthread_cond_init(&blockCond, nullptr);
    assert(success == 0);
}

// ----------------

Tasking::Signaler::~Signaler(void)
{
    pthread_cond_destroy(&blockCond);
}

// ----------------

void
Tasking::Signaler::wait()
{
    do
    {
        pthread_cond_wait(&blockCond, &blockMutex);
    } while (!wakeUp);
    wakeUp = false;
}

// ----------------

void
Tasking::Signaler::signal(void)
{
    wakeUp = true;
    pthread_cond_signal(&blockCond);
}
