/*
 * taskInput_impl.h
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

#ifndef INCLUDE_IMPL_TASKINPUT_IMPL_H_
#define INCLUDE_IMPL_TASKINPUT_IMPL_H_

// Standard integer definitions are needed to get UINTMAX_MAX
#include <limits>

#include "../taskInput.h"
#include "../taskUtils.h"

namespace Tasking
{

class Input;
class Channel;
class Task;
class TaskImpl;

struct InputImpl
{

    /**
     * Initialize the API part of the implementation.
     *
     * @param input Reference to the input which is implemented by this structure.
     */
    InputImpl(Input& input);

    /**
     * Getter method to access the task channel associated with this input
     * @result Pointer to the task channel associated with the this input
     */
    Channel* getChannel(void) const;

    /**
     * Call to notify an input. A call to this method can lead into an activation of the associated
     * task.
     */
    void notifyInput(void);

    /// Reference to the input implemented by the structure.
    Input& parent;

    /// Pointer to the task the channel is associated to.
    TaskImpl* m_task;

    /// The with the input associated channel.
    Channel* m_channel;

    /**
     * Flag to indicate if the input is final. If true, an activation of the input will trigger
     * the corresponding task immediately without respect to other inputs.
     */
    bool m_final;

    /// Flag to indicate synchronization mode
    bool m_synchron;

    /// Number of notifications since last reset
    volatile unsigned int m_notifications;

    /// Flag to count missed notifications in synchronization mode
    volatile unsigned int m_missedNotifications;

    /// Threshold of activation to activate the input
    unsigned int m_activationThreshold;

    /// Mutex to synchronize asynchron calls in synchronization mode
    Mutex m_mutex;

    /// Next input in the list of associated inputs of a channel
    InputImpl* channelNextInput;

    /// Flag to indicate the input as uninitialized
    static const unsigned int uninitialized = std::numeric_limits<unsigned int>::max();
};

} // namespace Tasking

#endif /* INCLUDE_IMPL_TASKINPUT_IMPL_H_ */
