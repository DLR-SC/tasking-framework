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

#include "mutex.h"
#include <iostream>

Tasking::Mutex::Mutex(void) : occupied(false)
{
}

void
Tasking::Mutex::enter(void)
{
    std::clog << "Tasking::Mutex::enter " << this;
    if (occupied)
    {
        std::clog << " Error: Reenter mutex";
    }
    std::clog << std::endl;
    occupied = true;
}

void
Tasking::Mutex::leave(void)
{
    std::clog << "Tasking::Mutex::leave " << this;
    if (!occupied)
    {
        std::clog << " Error: Leaving mutex without entering";
    }
    std::clog << std::endl;
    occupied = false;
}
