/*
 * mutex.h
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

#ifndef TASKING_INCLUDE_ARCH_LINUX_MUTEX_H_
#define TASKING_INCLUDE_ARCH_LINUX_MUTEX_H_

#include <pthread.h>

namespace Tasking
{

/// Class interface for POSIX pthread mutexes
class Mutex
{
public:
    /// Initialize POSIX pthread mutex
    Mutex(void);
    /// Free POSIX phread mutex
    ~Mutex(void);

    /// Lock the POSIX pthread mutex
    void enter(void);
    /// Unlock the POSIX pthread mutex
    void leave(void);

protected:
    /// Mutex from the pthread library
    pthread_mutex_t blockMutex;
};

// --------- inlines ----------

inline void
Mutex::enter(void)
{
    pthread_mutex_lock(&blockMutex);
}

inline void
Mutex::leave(void)
{
    pthread_mutex_unlock(&blockMutex);
}

} // namespace Tasking

#endif /* TASKING_INCLUDE_ARCH_LINUX_MUTEX_H_ */
