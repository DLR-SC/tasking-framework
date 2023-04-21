/*
 * taskInput.cpp
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

#include "taskInput.h"
#include "taskChannel.h"
#include "task.h"

class UnprotectedChannelAccess : public Tasking::Channel
{
public:
    using Tasking::Channel::associateTo;
    using Tasking::Channel::deassociate;
    using Tasking::Channel::reset;
    using Tasking::Channel::synchronizeEnd;
    using Tasking::Channel::synchronizeStart;
};

Tasking::Input::Input(void) : impl(*this)
{
}

//-------------------------------------

void
Tasking::Input::configure(Channel& channel, unsigned int activations, bool final)
{
    // Check if input is associated currently, if it is connect, delete association first
    if (impl.m_channel != nullptr)
    {
        static_cast<UnprotectedChannelAccess*>(impl.m_channel)->deassociate(impl);
    }
    impl.m_channel = &channel;
    configure(activations, final);
    static_cast<UnprotectedChannelAccess*>(impl.m_channel)->associateTo(impl);
}

//-------------------------------------

void
Tasking::Input::configure(unsigned int activations, bool final)
{
    impl.m_activationThreshold = activations;
    impl.m_final = final;
    impl.m_synchron = (impl.m_activationThreshold > 0u); // For optional inputs no synchronization is available
}

//-------------------------------------

void
Tasking::Input::setSynchron(bool syncState)
{
    // For optional inputs, no synchronization is available
    impl.m_synchron = syncState && (impl.m_activationThreshold > 0u);
    // In one task run the task consumes in synchronous mode only the threshold value, in asynchronous mode all messages
    if (impl.m_synchron)
    {
        // When the input has enough notification to activate a task, ...
        if (impl.m_notifications > impl.m_activationThreshold)
        {
            // ... distribute the notifications on pending ones and the one which had activate the task.
            impl.m_missedNotifications = impl.m_notifications - impl.m_activationThreshold;
            impl.m_notifications = impl.m_activationThreshold;
        }
    }
    else
    {
        // In asynchronous mode there are no missed notifications.
        // Adjust values to the expected state as with all notifications in asynchronous mode.
        impl.m_notifications += impl.m_missedNotifications;
        impl.m_missedNotifications = 0u;
    }
}

//-------------------------------------

bool
Tasking::Input::associate(Channel& channel)
{
    impl.m_channel = &channel;
    return static_cast<UnprotectedChannelAccess*>(impl.m_channel)->associateTo(impl);
}

//-------------------------------------

void
Tasking::Input::deassociate(void)
{
    if (impl.m_channel != nullptr)
    {
        static_cast<UnprotectedChannelAccess*>(impl.m_channel)->deassociate(impl);
        impl.m_channel = nullptr;
    }
}

//-------------------------------------

void
Tasking::Input::connectTask(TaskImpl& task)
{
    impl.m_task = &task;
}

//-------------------------------------

void
Tasking::Input::reset(void)
{
    // If connected to a channel, than reset the channel
    if (impl.m_channel != nullptr)
    {
        static_cast<UnprotectedChannelAccess*>(impl.m_channel)->reset();
    }
    // When configured as synchronized, the missed activations has to be overtaken
    if (impl.m_synchron)
    {
        impl.m_mutex.enter();
        // Check if a new activation of a connected task should happen by the missed activation calls.
        if ((impl.m_missedNotifications >= impl.m_activationThreshold))
        {
            // Activation is necessary, so overtake the new bunch of activations and activate the task.
            impl.m_missedNotifications -= impl.m_activationThreshold;
            impl.m_notifications = impl.m_activationThreshold;
            impl.m_mutex.leave();
            impl.m_task->activate();
        }
        else
        {
            // No activation is necessary
            impl.m_notifications = impl.m_missedNotifications;
            impl.m_missedNotifications = 0u;
            impl.m_mutex.leave();
        }
    }
    else
    {
        // Not synchronized, only reset activations.
        impl.m_notifications = 0;
    }
}

//-------------------------------------

bool
Tasking::Input::isActivated(void) const
{
    bool isActive = false;

    // First case: optional input marked as final, activate only if push came
    if (impl.m_final && (impl.m_activationThreshold == 0))
    {
        isActive = (impl.m_notifications > 0);
    }
    else
    {
        isActive = (impl.m_notifications >= impl.m_activationThreshold);
    }
    return isActive;
}

//-------------------------------------

bool
Tasking::Input::isOptional(void) const
{
    return (impl.m_activationThreshold == 0);
}

//-------------------------------------

bool
Tasking::Input::isFinal(void) const
{
    return impl.m_final;
}

//-------------------------------------

bool
Tasking::Input::isValid(void) const
{
    return (impl.m_activationThreshold != impl.uninitialized) && (impl.m_channel != nullptr) &&
           (impl.m_task != nullptr);
}

//-------------------------------------

unsigned int
Tasking::Input::getNotifications(void) const
{
    return impl.m_notifications;
}

//-------------------------------------

unsigned int
Tasking::Input::getPendingNotifications(void) const
{
    return impl.m_missedNotifications;
}

//-------------------------------------

Tasking::Channel*
Tasking::InputImpl::getChannel(void) const
{
    return m_channel;
}

// ====================================

Tasking::InputImpl::InputImpl(Tasking::Input& api) :
    parent(api),
    m_task(nullptr),
    m_channel(nullptr),
    m_final(false),
    m_synchron(false),
    m_notifications(0),
    m_missedNotifications(0u),
    m_activationThreshold(uninitialized),
    channelNextInput(nullptr)
{
}

//-------------------------------------

void
Tasking::InputImpl::notifyInput(void)
{
    if (m_synchron)
    {
        m_mutex.enter();
        if (parent.isActivated())
        {
            ++m_missedNotifications;
            m_mutex.leave();
        }
        else
        {
            ++m_notifications;
            m_mutex.leave();
            if (parent.isActivated())
            {
                // Activation is reached, try to activate the task
                m_task->activate();
            }
        }
    }
    else
    {
        // Not synchronized
        ++m_notifications;
        if (parent.isActivated())
        {
            // Activation is reached, try to activate the task
            m_task->activate();
        }
    }
}

//-------------------------------------

void
Tasking::Input::synchronizeStart(void)
{
    if (impl.m_channel != nullptr)
    {
        static_cast<UnprotectedChannelAccess*>(impl.m_channel)
                ->synchronizeStart(&impl.m_task->parent, impl.m_notifications);
    }
}

//-------------------------------------

void
Tasking::Input::synchronizeEnd(void)
{
    if (impl.m_channel != nullptr)
    {
        static_cast<UnprotectedChannelAccess*>(impl.m_channel)->synchronizeEnd(&impl.m_task->parent);
    }
}
