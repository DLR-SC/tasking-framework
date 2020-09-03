/*
 * signaler.h
 *
 * Copyright 2012-2020 German Aerospace Center (DLR) SC
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

#ifndef TASKING_INCLUDE_EXAMPLE_CUSTOM_SIGNALER_H_
#define TASKING_INCLUDE_EXAMPLE_CUSTOM_SIGNALER_H_

#include "mutex.h"

namespace Tasking
{

/// Implementation of a signaler without functionality
class Signaler : public Mutex
{
public:
    /**
     * Wait until some other software component calls signal to the signaler. In this implementation it has no
     * functionality, except an assertion that wait is only called when the caller is in the mutex to support unit
     * tests.
     */
    void wait(void);

    /**
     * Give the signal to the signaler. One of the threads which has called wait will wake up, other still sleeping.
     * In this implementation it has no functionality, except an assertion that signal is only called when the caller is
     * in the mutex to support unit tests.
     */
    void signal(void);
};

} // namespace Tasking

#endif /* TASKING_INCLUDE_ARCH_NONE_SIGNALER_H_ */
