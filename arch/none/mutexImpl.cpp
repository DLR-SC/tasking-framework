/*
 * mutex.cpp
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

#include "mutexImpl.h"

#include <cassert>

Tasking::MutexImpl::MutexImpl(void) : occupied(false)
{
}

void
Tasking::MutexImpl::enter(void)
{
    assert(!occupied); // Reenter is not allowed
    occupied = true;
}

void
Tasking::MutexImpl::leave(void)
{
    assert(occupied); // Only leave mutex if in the mutex
    occupied = false;
}
