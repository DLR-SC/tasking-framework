/*
 * taskChannel.cpp
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

#include <taskChannel.h>
#include <taskInput.h>
#include <taskUtils.h>

Tasking::Channel::Channel(ChannelId channelId) : m_channelId(channelId), m_inputs(nullptr)
{
    // For channel identification 0 channel identification is number of instantiated channels yet.
    static ChannelId channelCount = 1u;
    if (channelId == 0)
    {
        m_channelId = channelCount;
        ++channelCount;
    }
}

//-------------------------------------

Tasking::Channel::Channel(const char* channelName) : m_channelId(getChannelIdFromName(channelName)), m_inputs(NULL)
{
}

//-------------------------------------

Tasking::Channel::~Channel(void)
{
}

//-------------------------------------

bool
Tasking::Channel::associateTo(Tasking::InputImpl& p_input)
{
    bool inputIsUnique = true;
    for (InputImpl* search = m_inputs; inputIsUnique && (search != nullptr); search = search->channelNextInput)
    {
        if (&p_input == search)
        {
            inputIsUnique = false;
        }
    }
    if (inputIsUnique)
    {
        // p_input becomes the new head of the input list.
        p_input.channelNextInput = m_inputs;
        m_inputs = &p_input;
    }
    return inputIsUnique;
}

//-------------------------------------

void
Tasking::Channel::deassociate(Tasking::InputImpl& p_input)
{
    // Check if input is head of the link list
    if (&p_input == m_inputs)
    {
        m_inputs = p_input.channelNextInput;
    }
    else
    {
        // Input to remove from list is somewhere in the list, search it and get the previous element
        InputImpl* previous = m_inputs;
        for (InputImpl* search = m_inputs->channelNextInput; (search != nullptr) && (&p_input != search);
             search = search->channelNextInput)
        {
            // Not found yet, so previous becomes the current one
            previous = search;
        }
        // Remove from list if really found
        if (&p_input == previous->channelNextInput)
        {
            previous->channelNextInput = p_input.channelNextInput;
        }
    }
}

//-------------------------------------

void
Tasking::Channel::push(void)
{
    for (InputImpl* i = m_inputs; i != NULL; i = i->channelNextInput)
    {
        i->notifyInput();
    }
}

//-------------------------------------

void
Tasking::Channel::synchronizeStart(const Task*, unsigned int)
{
    // Nothing to do
}

//-------------------------------------

void
Tasking::Channel::synchronizeEnd(Task*)
{
    // Nothing to do
}
//-------------------------------------

void
Tasking::Channel::reset()
{
    // Nothing to do
}

//-------------------------------------

Tasking::ChannelId
Tasking::Channel::getChannelId(void) const
{
    return m_channelId;
}

//-------------------------------------

void
Tasking::Channel::setChannelName(const char* newChannelName)
{
    m_channelId = getChannelIdFromName(newChannelName);
}

//-------------------------------------

void
Tasking::Channel::setChannelId(ChannelId newChannelId)
{
    m_channelId = newChannelId;
}
