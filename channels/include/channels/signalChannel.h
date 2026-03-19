/*
 * signalChannel.h
 *
 * Copyright 2025 German Aerospace Center (DLR) SC
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
#ifndef CHANNELS_INCLUDE_CHANNELS_SIGNAL_CHANNEL_H_
#define CHANNELS_INCLUDE_CHANNELS_SIGNAL_CHANNEL_H_

#include <taskChannel.h>

namespace Tasking
{

/**
 * SignalChannel is an empty channel for triggering inputs without sharing data.
 * This class should be used instead of Event when time is not required for triggering an input due to its better
 * performance (runtime and memory wise).
 * @see Event
 */
class SignalChannel : public Channel
{
public:
    /**
     * Instantiation when data has a default constructor.
     * @param channelId Identification of the channel.
     */
    explicit SignalChannel(const ChannelId channelId = 0);

    /**
     * Instantiation when data has a default constructor.
     * @param channelName Name of the channel. Only the first four characters are converted into a channel.
     * identification
     */
    explicit SignalChannel(const char* channelName);

    // Disabling copy constructor
    SignalChannel(const SignalChannel&) = delete;
    // Disabling assignment operator
    SignalChannel& operator=(const SignalChannel&) = delete;

    /**
     * Activate inputs assigned to this channel.
     */
    void trigger();
};
} // namespace Tasking

#endif /* CHANNELS_INCLUDE_CHANNELS_SIGNAL_CHANNEL_H_ */
