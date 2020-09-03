/*
 * taskInput.h
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

#ifndef TASKINPUT_H_
#define TASKINPUT_H_

#include <mutex.h>
#include "impl/taskInput_impl.h"

namespace Tasking
{

class Task;
class Channel;

/**
 * Manage the activation state of incoming channels to a task. If all task inputs of a task are activated
 * or at least one is activated and marked as final, the task will execute.
 * A task input is activated, if the number of activations reaches the activation threshold defined
 * by the constructor of the task input. Defining a task input with activation threshold of zero
 * means, that the input is only optional for a task and will not block task activation by other inputs.
 */
class Input
{
public:
    /**
     * Null initialization of a task input.
     */
    Input(void);

    /**
     * Destructor
     */
    virtual ~Input(void)
    {
    }

    /**
     * Connect the input to a channel and configure the behavior for the activation of the input.
     * Without this call, the input is invalid and an application can not start. As side effect
     * the input is configured as synchronous input. To get an unsynchronized input a call to
     * method setSynchron with parameter false is necessary.
     *
     * @param channel Reference to the channel where this input is associated to.
     *
     * @param activations Threshold value of new data notifications at channel to activate the task.
     * Default value is one incoming message to trigger a task. A value of 0 mark the task input
     * optional for the accepting tasks.
     *
     * @param final Flag to indicate that reaching the activation threshold activate the task
     * immediately without respect to other activation states of other inputs from the task.
     * Default value is false.
     */
    void configure(Channel& channel, unsigned int activations = 1, bool final = false);

    /**
     * Configure the settings of the input without setting a channel to the input. The input remains
     * invalid until a channel is associated to the input. As side effect the input is configured as
     * synchronous input. To get an unsynchronized input a call to method setSynchron with parameter
     * false is necessary.
     *
     * @param activations Threshold value of incoming messages on a channel to activate the task.
     * Default value is one incoming message to trigger a task. A value of 0 mark the task input
     * optional for the accepting tasks.
     *
     * @param final Flag to indicate that reaching the activation threshold triggers the task
     * immediately without respect to other activation states of other inputs from the task.
     * Default value is false.
     *
     * @see associate
     */
    void configure(unsigned int activations, bool final = false);

    /**
     * Configure input synchronization as on. If synchronization is on and the input is activated, the reset operation
     * consumes only the number of expected activations. No notifications are lost when the input is activated and the
     * reset operation is not executed for this activation cycle. If enough notifications have been received when the
     * reset operation is started, the input get's immediately activated directly after the reset operation.
     * E.g. if activations is set to two and five notifications happens without
     * the reset operation, the input is activated directly again by the reset operation. After the next
     * reset operation the input will wait for a further notification to get activated.
     *
     * By default the synchronization is switched on.
     *
     * @param syncState Setting for the synchronization state. If set to false notifications will be lost after the
     * activation of the input and before its reset operation is finalized. An associated channel can hold in this
     * case unread data items and the associated task has to handle these circumstance.
     */
    void setSynchron(bool syncState = true);

    /**
     * Connect a channel to the input. If the input is configured, it becomes valid after the call.
     *
     * @param channel Reference to the message where this input is associated to.
     *
     * @result true if the association succeed. false if the input is already associated to the channel.
     *
     * @see configure
     */
    bool associate(Channel& channel);

    /**
     * Remove the association between the input and the channel. The input is no longer notified by the channel
     * and can not be activated until a new association to a channel is set. The input becomes invalid
     * after the call.
     */
    void deassociate(void);

    /**
     * Connect the input with a task. By usage of a TaskProvider the method is called by the constructor.
     * The method is also called when instantiating a task or connect an input array to a task. By default
     * from application code no call is necessary.
     *
     * @see Task::construct
     * @see InputArray::connectTask
     */
    void connectTask(TaskImpl& task);

    /**
     * Reset the activation state to 0 activations.
     */
    virtual void reset(void);

    /**
     * Request if the task input is notified the expected number of times since the last reset.
     *
     * @result True, if the task input is activated. False if not the required number of notification
     * happens. For optional and final inputs the result is false if not at least one notification happens.
     */
    bool isActivated(void) const;

    /**
     * Check if the input is marked as final.
     *
     * @result True, if the input is marked as final. False if not.
     */
    bool isFinal(void) const;

    /**
     * Check if the input is configured as optional
     *
     * @result True if the input is configured with zero arrival as activation threshold, else false.
     */
    bool isOptional(void) const;

    /**
     * True if the input is correctly configured.
     * @see configure
     */
    bool isValid(void) const;

    /**
     * Request the number of activations since last reset of the task input.
     * Special case: optional final input returns only true if an activation came
     *
     * @result Number of activations since last call to reset.
     */
    unsigned int getActivations(void) const;

    /**
     * Type safe request of a channel from a task input
     * @tparam ChannelType Type of the channel to request
     * @result Pointer of corresponding task channel type associated with the this input
     */
    template<typename ChannelType>
    ChannelType*
    getChannel(void) const
    {
        return static_cast<ChannelType*>(impl.getChannel());
    }

protected:
    /**
     * The associated task start to execute. This method is protected by the scheduler against concurrent
     * access of two tasks associated with the scheduler.
     */
    virtual void synchronizeStart(void);

    /**
     * The associated task has finalize its run. This method is protected by the scheduler against concurrent
     * access of two tasks associated with the scheduler.
     */
    virtual void synchronizeEnd(void);

private:
    /// Implementation part of the input
    InputImpl impl;
};
} // namespace Tasking

#endif /* TASKINPUT_H_ */
