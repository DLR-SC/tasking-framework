/*
 * signaler.cpp
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

#include <iostream>
#include "signaler.h"

void
Tasking::Signaler::wait()
{
    std::clog << "Tasking::Signaler::wait " << this;
    if (!Mutex::occupied)
    {
        std::clog << " Error: wait without entering mutex";
    }
    std::clog << std::endl;
}

void
Tasking::Signaler::signal(void)
{
    std::clog << "Tasking::Signaler::signal " << this;
    if (!Mutex::occupied)
    {
        std::clog << " Error: signal without entering mutex";
    }
    std::clog << std::endl;
}
