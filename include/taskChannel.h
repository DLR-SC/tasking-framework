/*
 * taskChannel.h
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

#ifndef TASKCHANNEL_H_
#define TASKCHANNEL_H_

#include "taskTypes.h"

namespace Tasking
{

// Forward declaration for input and task.
class InputImpl;
class Task;

/**
 * A task channel is the base for a data container to store data and distribute this data to
 * all tasks in a system. It can be associated to several task inputs. With the push operation it
 * notifies all associated inputs that new data is available, which can result in an activation
 * of a task reading the data for further computation steps.
 *
 * In an implementation the class should be specialized for a data container with the needed synchronization
 * capabilities. The task channel provides some entry point to perform synchronization without running
 * conditions between all executors of a scheduler instance.
 *
 * An identifier can be assigned to the task channel. This identification is not not needed for the Tasking
 * Framework, but by the extension of the tasking framework on distributed systems. The identifier can be _either_
 * a numeric id or a name of up to 4 characters in length. Only use the respective setter/getter.
 *
 * @see TaskInput
 */
class Channel
{
protected:
    /// Identification of the channel. For debugging purposes it should always be the first element of the channel.
    ChannelId m_channelId;

public:
    /**
     * Initialize a task channel.
     *
     * @param channelId Identifier for this channel. For default value 0 the number of instantiated channels
     *                    becomes the identification.
     *
     * NOTE:
     *    It is the responsibility of the user to ensure uniqueness of the channel and events identifications.
     */
    Channel(ChannelId channelId = 0);

    /**
     * Initialize a named task channel.
     *
     * @param channelName Null-terminated string specifying a name for this channel. The name will be
     *                    truncated after 4 characters.
     * NOTE:
     *    It is the responsibility of the user to ensure uniqueness of the channel and events identifications.
     */
    Channel(const char* channelName);

    /**
     * Destructor needed by virtual methods
     */
    virtual ~Channel(void);

    /**
     * Enquire the identification of a task channel.
     *
     * @result The identification of type ChannelId for the task channel.
     *
     * NOTE:
     *    If a channel name was assigned to this channel the identification
     *    will represent the numeric value of the 4-character string.
     *
     * @see ChannelId
     * @see convertChannelIdToString
     */
    ChannelId getChannelId(void) const;

    /**
     * Set a new name for a task channel.
     *
     * @param newChannelName Null-terminated string specifying the new name
     *                       which will be set for the task channel.
     *                       The name will be truncated after 4 characters.
     *
     */
    void setChannelName(const char* newChannelName);

    /**
     * Set a new identification for a task channel.
     *
     * @param newChannelId The new ID which will be set for the task channel.
     *
     * @see ChannelId
     */
    void setChannelId(ChannelId newChannelId);

protected:
    /**
     * Establish an association to an input. This method is called when an input is configured.
     * It should not be called by the application code, instead use application methods of an input.
     *
     * @param p_input Reference to the input which shall associated to this class.
     *
     * @result true if the association succeed. false if the input is already associated to the channel.
     *
     * @see Input
     */
    bool associateTo(InputImpl& p_input);

    /**
     * Remove the association between the channel and an input. This method is called when inputs are
     * reconfigured.
     * The method should not be called from application code, instead use the configuration commands from input.
     *
     * @param p_input Pointer to the input which association shall be removed from this class.
     *
     * @see Input
     */
    void deassociate(InputImpl& p_input);

    /**
     * Finalize a push operation. It should always be called by the specialization of a channel with a data container
     * whenever new data is published. The method notifies the availability of new data to all associated inputs.
     */
    void push(void);

    /**
     * A task which expects data from this channel is started. A specialization as data container can override
     * the method with own synchronization stuff. The call is synchronized by the associated scheduler of the started
     * task, so that two tasks with the same scheduler can not call this method concurrently.
     *
     * @param p_task Pointer to the task which starts to work on the data. It can be used as key to identify a
     *                 specific task.
     *
     * @param volume Number of activations of the associated task input.
     *
     * @see synchronizeEnd
     */
    virtual void synchronizeStart(const Task* p_task, unsigned int volume = 1);

    /**
     * A task which expects data from this channel has to finalize its run. A specialization as data container can
     * override the method with own synchronization stuff. The call is synchronized by the associated scheduler
     * of the finalized task, so that two tasks with the same scheduler can not call this method concurrently.
     *
     * @param p_task Pointer to the task which ends the work on the data. It can be used as key to identify a
     *                 specific task, for example to match them to the task start announced by synchronizeStart.
     *
     * @see synchronizeStart
     */
    virtual void synchronizeEnd(Task* p_task);

    /**
     * Reset the channel. The method is called whenever a task, which expects data from this channel, is finalized
     * or all tasks of a group the task belongs to are finalized. A specialization as data container can override
     * the method with further steps on data management. In multi-threaded system a concurrent call of the method
     * is possible. The default implementation reset all associated inputs, which is necessary for the next start
     * of associated tasks.
     */
    virtual void reset(void);

private:
    /// Head of list of task inputs associated to this channel.
    InputImpl* m_inputs;
};

} // namespace Tasking

#endif /* TASKCHANNEL_H_ */
