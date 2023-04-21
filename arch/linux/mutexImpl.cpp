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

Tasking::MutexImpl::MutexImpl(void)
{
    int success;
    success = pthread_mutex_init(&blockMutex, nullptr);
    assert(success == 0);
}

Tasking::MutexImpl::~MutexImpl(void)
{
    pthread_mutex_destroy(&blockMutex);
}
