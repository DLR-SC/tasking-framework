/*
 * taskBarrier.cpp
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

#include <taskBarrier.h>

namespace Tasking
{

Tasking::Mutex monitor;

} // namespace Tasking

// ------------------------------------

Tasking::Barrier::Barrier(unsigned int resetValue) : counter(resetValue), startValue(resetValue)
{
}

// ------------------------------------

void
Tasking::Barrier::increase(const unsigned int delta)
{
    protection.enter();
    counter += delta;
    protection.leave();
}

// ------------------------------------

void
Tasking::Barrier::push(void)
{
    protection.enter();
    if (counter > 0)
    {
        --counter;
        if (counter == 0)
        {
            Channel::push();
        }
    }
    protection.leave();
}

// ------------------------------------

void
Tasking::Barrier::reset(void)
{
    protection.enter();
    counter = startValue;
    protection.leave();
}
