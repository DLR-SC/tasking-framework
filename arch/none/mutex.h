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

#ifndef TASKING_INCLUDE_ARCH_NONE_MUTEX_H_
#define TASKING_INCLUDE_ARCH_NONE_MUTEX_H_

namespace Tasking
{

/// Mutex implementation without any functionality, e.g. for non concurrent software
class Mutex
{
public:
    /// Initialize test on occupancy
    Mutex(void);
    /// Enter the critical region. The method assert on entering an occupied mutex.
    void enter(void);
    /// Leave the critical region. The method assert on leaving an non occupied mutex.
    void leave(void);

protected:
    /// Flag to indicate mutex is occupied, used for assertion of reenter by unit tests
    bool occupied;
};
} // namespace Tasking

#endif /* TASKING_INCLUDE_ARCH_NONE_MUTEX_H_ */
