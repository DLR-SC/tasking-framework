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

#ifndef INCLUDE_TASKUTILS_H_
#define INCLUDE_TASKUTILS_H_

#include "taskTypes.h"
#include <mutex.h>

namespace Tasking
{

/**
 * Convert a name into a task identification.
 * @param name String with the name to convert.
 * @result TaskId computed from the name.
 */
TaskId getTaskIdFromName(const char* name);

/**
 * Convert a task identification to a name.
 * @param id Task identification to convert.
 * @param buffer Pointer to buffer to hold the resulting task name.
 * @param length Size of the buffer for task name. Optional and buffer size is assumed with five bytes. For buffer
 * with less than five bytes it is mandatory to set the value. The resulting string is shorten to the length.
 * @return Pointer to the name of the task with the corresponding identification. Pointer is identical to @a buffer.
 */
char* convertTaskIdToString(TaskId id, char* buffer, size_t length = 5);

/**
 * Convert a name into a task identification.
 * @param name String with the name to convert.
 * @result TaskId computed from the name.
 */
inline ChannelId
getChannelIdFromName(const char* name)
{
    return static_cast<ChannelId>(getTaskIdFromName(name));
}

/**
 * Convert a channel id to a name.
 * @param id Task identification to convert.
 * @param buffer Pointer to buffer to hold the resulting task name.
 * @param length Size of the buffer for task name. Optional and buffer size is assumed with five bytes.For buffer
 * with less than five bytes it is mandatory to set the value. The resulting string is shorten to the length.
 * @return Pointer to the name of the task with the corresponding identification. Pointer is identical to @a buffer.
 */
inline char*
convertChannelIdToString(ChannelId id, char* buffer, size_t length = 5)
{
    return convertTaskIdToString(static_cast<TaskId>(id), buffer, length);
}

/**
 * Guard for a code block as protected region. Entering to mutex when constructed, and leaving at end of block.
 */
class MutexGuard
{
public:
    /**
     * Enter into protected region.
     * @param mutex Reference to a mutex which protect the region.
     */
    MutexGuard(Tasking::Mutex& mutex);

    /// Leave protected region.
    ~MutexGuard();

private:
    /// Reference to the mutex
    Tasking::Mutex& mutex;
};

} // namespace Tasking

#endif /* INCLUDE_TASKUTILS_H_ */
