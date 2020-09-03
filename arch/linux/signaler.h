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

#ifndef TASKING_INCLUDE_ARCH_LINUX_SIGNALER_H_
#define TASKING_INCLUDE_ARCH_LINUX_SIGNALER_H_

#include "mutex.h"

namespace Tasking
{

/// A signaler based on the POSIX pthread conditional variables.
class Signaler : public Mutex
{
public:
    /// Initialize POSIX pthread conditional variable
    Signaler(void);

    /// Free POSIX pthread conditional variable
    ~Signaler(void);

    /**
     * Wait until another concurrent software component calls the signal method of these signaler. When the method
     * is left, the wake up flag will be false. The method should only called after the signaler is locked.
     * @see signal
     */
    void wait(void);

    /**
     * Give the signal to the signaler. One of the threads which has called wait will wake up, other still sleeping.
     * Signal the POSIX pthread conditional variable. When the call returns the wake up flag is true until the waiting
     * thread is waked up.
     * @see wait
     */
    void signal(void);

protected:
    /// Conditional from POSIX pthread library
    pthread_cond_t blockCond;

    /// Variable to check if a wake up from a conditional wait comes from signal call and not from another reason.
    bool wakeUp;
};

} // namespace Tasking

#endif /* TASKING_INCLUDE_ARCH_LINUX_SIGNALER_H_ */
