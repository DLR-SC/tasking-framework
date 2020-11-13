/*
 * taskUtils.cpp
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

#include <taskUtils.h>
#include <mutex.h>

Tasking::TaskId
Tasking::getTaskIdFromName(const char* name)
{
    TaskId id = 0u;
    if (name != NULL)
    {
        // Copy the new task name. It can be at most 4 characters long
        // and is assumed to be null-terminated
        size_t i = 0u;
        while ((i < 4u) && (name[i] != '\0'))
        {
            id = (id << 8u) + static_cast<unsigned int>(name[i]);
            ++i;
        }
        id = id << (8u * (4u - i));
    }
    return id;
}

// ------------------------------------

char*
Tasking::convertTaskIdToString(TaskId id, char* buffer, size_t length)
{
    if ((length > 1) && (buffer != NULL))
    {
        // Loop when we are inside the buffer length (excluding null termination) and not all bits converted
        for (unsigned int pos = 0u, shift = 32u; (pos < length - 1) && (shift > 0u); ++pos)
        {
            shift -= 8u; // Reduce shift first, so end check on bigger than zero becomes possible
            buffer[pos] = (id >> shift) & 0xFFu;
        }
        // Minimum between length-1 and 4 is the null termination. If name has fewer characters the lower bits are 0.
        buffer[(length > 5) ? 4 : length - 1] = 0;
    }
    return buffer;
}

// ====================================

Tasking::MutexGuard::MutexGuard(Tasking::Mutex& inMutex) : mutex(inMutex)
{
    mutex.enter();
}

// ------------------------------------

Tasking::MutexGuard::~MutexGuard()
{
    mutex.leave();
}
