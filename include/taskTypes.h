/*
 * taskTypes.h
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

#ifndef TASKTYPES_H_
#define TASKTYPES_H_

#include <cstdlib>
#include <cstddef>
#include <stdint.h>
#include <limits>

namespace Tasking
{

/// Type to express a time in milliseconds. It can be a time point or time span.
typedef uint64_t Time; // Over 500 million years are addressable without overflow

/// Constant to specify end of time
static const Time endOfTime = std::numeric_limits<Time>::max();

/// Type to express the task ID.
typedef uint32_t TaskId;

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
 * @return Pointer to the name of the task with the corresponding identification. Pointer is identical to @ref buffer.
 */
char* convertTaskIdToString(TaskId id, char* buffer, size_t length = 5);

/// Type to express the channel ID.
typedef uint32_t ChannelId;

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
 * @return Pointer to the name of the task with the corresponding identification. Pointer is identical to @ref buffer.
 */
inline char*
convertChannelIdToString(ChannelId id, char* buffer, size_t length = 5)
{
    return convertTaskIdToString(static_cast<TaskId>(id), buffer, length);
}

} // namespace Tasking

#endif /* TASKTYPES_H_ */
