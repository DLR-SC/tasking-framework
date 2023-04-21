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
#include <mutexImpl.h>

#include "impl/taskUtils_impl.h"

namespace Tasking
{

class Channel;
class Task;

/**
 * Convert a name into a task identification.
 * @param name String with the name to convert.
 * @result TaskId computed from the name.
 */
inline TaskId
getTaskIdFromName(const char* name)
{
    return getIdentificationFromName<TaskId>(name);
}

/**
 * Convert a task identification to a name.
 * @param id Task identification to convert.
 * @param buffer Pointer to buffer to hold the resulting task name.
 * @param length Size of the buffer for task name. Optional and buffer size is assumed with five bytes. For buffer
 * with less than five bytes it is mandatory to set the value. The resulting string is shorten to the length.
 * @return Pointer to the name of the task with the corresponding identification. Pointer is identical to @a buffer.
 */
inline char*
convertTaskIdToString(TaskId id, char* buffer, size_t length = sizeof(TaskId) + 1u)
{
    return convertIdentificationToString(id, buffer, length);
}

/**
 * Convert a name into a task identification.
 * @param name String with the name to convert.
 * @result TaskId computed from the name.
 */
inline ChannelId
getChannelIdFromName(const char* name)
{
    return getIdentificationFromName<ChannelId>(name);
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
convertChannelIdToString(ChannelId id, char* buffer, size_t length = sizeof(ChannelId) + 1u)
{
    return convertIdentificationToString(id, buffer, length);
}

/**
 * Class to request the name of a task or channel
 * Usage will be IdConverter(channel).name.
 */
class IdConverter
{
public:
    /**
     * Initialize the ID converter by a readout and conversion of the channel name
     * @param channel Reference to the channel to read out the name
     */
    IdConverter(const Tasking::Channel& channel);

    /**
     * Initialize the ID converter by a readout and conversion of the task name
     * @param tasl Reference to the task to read out the name
     */
    IdConverter(const Tasking::Task& task);

    // Name of the channel or task.
    char name[((sizeof(TaskId) < sizeof(ChannelId)) ? sizeof(TaskId) : sizeof(ChannelId)) + 1];
};

/**
 * Inline wrapper class to the Mutex implementation
 */
class Mutex : protected MutexImpl
{
public:
    /// Enter into the critical region protected by the Mutex
    void enter(void);
    /// Leave the critical region protected by the Mutex
    void leave(void);
};
inline void
Mutex::enter(void)
{
    MutexImpl::enter();
}
inline void
Mutex::leave(void)
{
    MutexImpl::leave();
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
