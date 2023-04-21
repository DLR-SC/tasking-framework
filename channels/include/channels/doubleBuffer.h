/*
 * doubleBuffer.h
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
#ifndef CHANNELS_INCLUDE_CHANNELS_DOUBLEBUFFER_H_
#define CHANNELS_INCLUDE_CHANNELS_DOUBLEBUFFER_H_

#include <type_traits>

#include <taskChannel.h>

namespace Tasking
{

/**
 * Implementation of a channel with a double buffer. This channel shall be used when only one writing tasks exists.
 * If more than one writing task exists, the application programmer is responsible to synchronize the writing tasks.
 * A send operation will go into the back buffer and the buffer is switched over when the data is copied.
 *
 * @tparam T Data type of the channel. The data type needs a default constructor and shall support an assignment
 * operator for a correct operation. Therefore, C arrays cannot be used.
 */
template<typename T>
class DoubleBuffer : public Channel
{
public:
    /**
     * Default Constructor.
     *
     * @param channelId Identification of the channel.
     */
    DoubleBuffer(const ChannelId channelId = 0);

    /**
     * Default Constructor.
     *
     * @param channelId Identification of the channel.
     */
    DoubleBuffer(const char* channelName);

    /**
     * Constructor with assignment of initial data in the buffer.
     *
     * @param initialValue Data for both elements in double buffer.
     * @param channelId Identification of the channel.
     */
    DoubleBuffer(const T initialValue, const ChannelId channelId = 0);

    /**
     * Constructor with assignment of initial data in the buffer and a channel name.
     *
     * @param initialValue Data for both elements in double buffer.
     * @param channelName Name of the channel.
     */
    DoubleBuffer(const T initialValue, const char* channelName);

    /**
     * Read data from the channel.
     *
     * @result Reference to the valid internal data buffer element.
     */
    const T& read(void) const;

    /**
     * Send data over the channel.
     *
     * @data Data to send.
     */
    void send(const T data);

    /**
     * Send data over the channel.
     * @data Data to send. If the pointer corresponds to the internal data buffer. In case of the allocated pointer,
     * no copy operation will be initiated, only the back buffer is switched over.
     *
     * @see allocate
     */
    void send(const T* data);

    /**
     * Get the pointer to the internal back buffer. This buffer can be used in combination with send to avoid
     * copy operations on big data structures. It is the responsibility of the application programmer to synchronize
     * the access to the data buffer between writer and reader task, if the reader task works on the reference.
     * (read@receiverTask, send@senderTask, send@senderTask, allocate@senderTask deliver pointer on data element the
     * receiver works on)
     *
     * @return Pointer to the internal data buffer.
     *
     * @see send
     */
    T* getBuffer(void);

private:
    /// Constant for the dimension of the buffer
    static const size_t bufferSize = 2;

    /**
     * Initialize buffer with data. Called by constructors with initialization.
     *
     * @param initialValue Data for both elements in double buffer.
     */
    void initBuffer(const T initialValue);

protected:
    /// Buffer for two data elements
    T data[bufferSize];

    /// Index to current back buffer data element
    size_t backIndex;
};

// ----- Implementation part -----

template<typename T>
DoubleBuffer<T>::DoubleBuffer(const ChannelId channelId) : Channel(channelId), backIndex(1u)
{
    static_assert(std::is_default_constructible<T>::value, "Type needs to be default constructible");
}

// ------------------------------------

template<typename T>
DoubleBuffer<T>::DoubleBuffer(const char* channelName) : Channel(channelName), backIndex(1u)
{
    static_assert(std::is_default_constructible<T>::value, "Type needs to be default constructible");
}

// ------------------------------------

template<typename T>
DoubleBuffer<T>::DoubleBuffer(const T initialValue, const ChannelId channelId) : Channel(channelId), backIndex(1u)
{
    static_assert(std::is_default_constructible<T>::value, "Type needs to be default constructible");
    initBuffer(initialValue);
}

template<typename T>
DoubleBuffer<T>::DoubleBuffer(const T initialValue, const char* channelName) : Channel(channelName), backIndex(1u)
{
    static_assert(std::is_default_constructible<T>::value, "Type needs to be default constructible");
    initBuffer(initialValue);
}

// ------------------------------------

template<typename T>
void
DoubleBuffer<T>::initBuffer(const T initialValue)
{
    static_assert(std::is_copy_assignable<T>::value, "Type needs an assignment operator");
    for (size_t index = 0u; index < bufferSize; ++index)
    {
        data[index] = initialValue;
    }
}

// ------------------------------------

template<typename T>
const T&
DoubleBuffer<T>::read(void) const
{
    // Determine valid index.
    size_t validIndex = (backIndex + 1u) % bufferSize;

    return data[validIndex];
}

// ------------------------------------

template<typename T>
void
DoubleBuffer<T>::send(const T inData)
{
    // Call convert it to data so reference call can be used.
    data[backIndex] = inData;
    // Switch over buffers.
    backIndex = (backIndex + 1u) % bufferSize;

    push();
}

// ------------------------------------

template<typename T>
void
DoubleBuffer<T>::send(const T* inData)
{
    // Check if inData is not back buffer. If it is not back buffer, copy data into backbuffer
    if (inData != data + backIndex)
    {
        data[backIndex] = *inData;
    }
    // Switch over buffers.
    backIndex = (backIndex + 1u) % bufferSize;

    push();
}

// ------------------------------------

template<typename T>
T*
DoubleBuffer<T>::getBuffer(void)
{
    return (data + backIndex);
}

} // namespace Tasking

#endif /* CHANNELS_INCLUDE_CHANNELS_DOUBLEBUFFER_H_ */
