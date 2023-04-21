/*
 * singleBufferChannel.h
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
#ifndef CHANNELS_INCLUDE_CHANNELS_SINGLEBUFFER_H_
#define CHANNELS_INCLUDE_CHANNELS_SINGLEBUFFER_H_

#include <type_traits>

#include <taskChannel.h>

namespace Tasking
{

/**
 * Implementation of a channel with a single buffer. This channel should only be used when sending task and receiving
 * task are synchronized, e.g. by a group or the sender task is synchronized with one or more channels back from the
 * receiver task.
 * @tparam T Data type of the channel. Data type shall provide a copy operator.
 */
template<typename T>
class SingleBuffer : public Channel
{
public:
    /**
     * Instantiation when data has a default constructor.
     * @param channelId Identification of the channel.
     */
    SingleBuffer(const ChannelId channelId = 0);

    /**
     * Instantiation when data has a default constructor.
     * @param channelName Name of the channel. Only the first four characters are converted into a channel.
     * identification
     */
    SingleBuffer(const char* channelName);

    /**
     * Constructor with assignment of initial data in the buffer.
     *
     * @param initialValue Value to initialize the buffer element.
     * @param channelId Identification of the channel.
     */
    SingleBuffer(const T initialValue, const ChannelId channelId = 0);

    /**
     * Constructor with assignment of initial data in the buffer.
     *
     * @param initialValue Value to initialize the buffer element.
     * @param channelName Name of the channel. Only the first four characters are converted into a channel.
     * identification
     */
    SingleBuffer(const T initialValue, const char* channelName);

    /**
     * Read data from the channel.
     *
     * @result Reference to the internal data buffer.
     */
    const T& read(void) const;

    /**
     * Send data to the channel and push them.
     *
     * @data Data to send.
     */
    void send(const T data);

    /**
     * Send data to the channel and push them.
     *
     * @data Data to send. If the pointer corresponds to the internal data buffer, requested with getBuffer, no copy
     * operation will be initiated.
     *
     *
     * @see getBuffer
     */
    void send(const T* data);

    /**
     * Get the pointer to the internal buffer. This buffer can be used in combination with the method send to avert
     * copy operations on big data structures. It is the responsibility of the application programmer to synchronize
     * the access to the data buffer between writer and reader tasks.
     *
     * @return Pointer to the internal data buffer.
     *
     * @see send
     */
    T* getBuffer(void);

protected:
    T data;
};

// ----- Implementation part -----

template<typename T>
SingleBuffer<T>::SingleBuffer(const ChannelId channelId) : Tasking::Channel(channelId)
{
}

// ------------------------------------

template<typename T>
SingleBuffer<T>::SingleBuffer(const char* channelName) : Tasking::Channel(channelName)
{
}

// ------------------------------------

template<typename T>
SingleBuffer<T>::SingleBuffer(const T initialValue, const ChannelId channelId) : Channel(channelId), data(initialValue)
{
}

// ------------------------------------

template<typename T>
SingleBuffer<T>::SingleBuffer(const T initialValue, const char* channelName) : Channel(channelName), data(initialValue)
{
}

// ------------------------------------
template<typename T>
const T&
SingleBuffer<T>::read(void) const
{
    return data;
}

// ------------------------------------

template<typename T>
void
SingleBuffer<T>::send(const T inData)
{
    static_assert(std::is_copy_assignable<T>::value, "Type needs a copy operator");

    // Call convert it to data so reference call can be used.
    data = inData;
    push();
}

// ------------------------------------

template<typename T>
void
SingleBuffer<T>::send(const T* inData)
{
    if (inData != &data)
    {
        data = *inData;
    }
    push();
}

// ------------------------------------

template<typename T>
T*
SingleBuffer<T>::getBuffer(void)
{
    return &data;
}

} // namespace Tasking

#endif /* CHANNELS_INCLUDE_CHANNELS_SINGLEBUFFER_H_ */
