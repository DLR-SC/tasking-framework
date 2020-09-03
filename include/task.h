/*
 * task.h
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

#ifndef TASK_H_
#define TASK_H_

#include "impl/task_Impl.h"
#include "taskTypes.h"

namespace Tasking
{

/**
 * A task performs a single execution if all inputs of the input array are activated or one input
 * marked as final is activated. To implement the body of the task, the method execute has
 * to be overridden. To simplify creating of a task with all its inputs the template class
 * TaskProvider exists, which provides an instance of the input array for all incoming inputs of
 * the task.
 *
 * The purpose of this class is the reactive and concurrent processing on incoming events or data
 * packages. For example a task implementation can be the reaction on an interrupt distributed by
 * an interrupt channel, the classification of an incoming message on a channel, or a further
 * computation step in a sequence of computation tasks.
 *
 * For a correct operation it is necessary to configure the class correctly. This means the inputs
 * are configured with the expected settings and connected to a channel and the inputs in the input
 * array are connected to this task by a call of the method construct or by using the template
 * class TaskProvider instead of Task directly.
 *
 * To combine several tasks to a group, the tasks should bind to a group with the class Group.
 * By default each task is scheduled without relationships to other task, which means that the
 * method reset is called directly after the task is executed and its inputs are synchronized. If
 * the task is bind to a group reset is called only when all tasks associated to a group are marked
 * as executed. By this, a subsequent activation can only happen when all tasks of the group are
 * executed.
 *
 * Each task has an identifier which shall unique. It can be _either_ a numeric id or a name of up to
 * 4 characters in length. Only use the respective setter/getter methods.
 *
 * @see TaskProvider
 * @see Group
 * @see InputArray
 * @see Event
 * @see Channel
 */
class Task
{
protected:
    /**
     * The identification of the task. It should always mapped to the first data member to find the
     * identification easy in a memory dump.
     */

    TaskId m_taskId;

public:
    /**
     * First initialization step and connect the task to the scheduler. The task is not fully
     * initialized until the second initialization step with a call to construct is done.
     *
     * @param scheduler Reference to the scheduler. It provide means to execute this task.
     *
     * @param policy Reference to the data structure needed for management of the task by the scheduler.
     *
     * @param inputs Reference to an array of inputs associated with this task.
     *
     * @param taskId Identification of the task. This identification is needed by extensions of the
     *                 Tasking framework to address a task or to identify the task for debugging. If not given,
     *                 an identification the number of constructor calls is given as identification.
     *
     * NOTE:
     *    It is the responsibility of the user to ensure uniqueness of the task id.
     *
     * @see taskId_t
     * @see construct
     */
    Task(Scheduler& scheduler, SchedulePolicy::ManagementData& policy, InputArray& inputs, TaskId taskId = 0u);

    /**
     * First initialization of task with a task name. The task is not fully initialized until the
     * second initialization step with a call to construct is done.
     *
     * @param scheduler Reference to the scheduler which performs this task.
     *
     * @param policy Reference to the data structure needed for management of the task by the scheduler.
     *
     * @param inputs Reference to an array of inputs associated with this task.
     *
     * @param taskName Null-terminated string specifying a name for this task. The name will be
     *                    truncated after 4 characters. Only a name _or_ a taskId can be used for
     *                    channel identification.
     * @see construct
     */
    Task(Scheduler& scheduler, SchedulePolicy::ManagementData& policy, InputArray& inputs, const char* taskName);

    /// Destructor needed by virtual methods
    virtual ~Task();

    /**
     * Connect a channel to an input of the task.
     *
     * @param key Identifications of the input which should connect to the channel.
     *
     * @param channel Reference to the channel to connect. The channel should have the type the task expect.
     *
     * @result true if the configuration of the input to the channel succeed. false if an error during the configuration
     * happened.
     */
    bool configureInput(unsigned int key, Channel& channel);

    /// @result True if all inputs are configured and connected to a channel.
    bool isValid(void) const;

    /**
     * A call resets the activation state of all task inputs. This method is called whenever a task was executed
     * by the associated scheduler or when the task belongs to a group all tasks of the group are executed.
     */
    virtual void reset(void);

    /**
     * Enquire the identification of a task
     *
     * @result The identification of type taskId_t for the task.
     *
     * NOTE:
     *    If a task name was assigned to this task the id
     *    will represent the numeric value of the 4-character string.
     *
     * @see taskId_t
     * @see convertTaskIdToString
     */
    TaskId getTaskId(void) const;

    /**
     * Set a new name for a task
     *
     * @param newTaskName Null-terminated string specifying the new name
     *                    which will be set for the task.
     *                    The name will be truncated after 4 characters.
     *
     */
    void setTaskName(const char* newTaskName);

    /**
     * Set a new ID for a task
     * @param newTaskId The new ID which will be set for the task.
     *
     * @see taskId_t
     */
    void setTaskId(TaskId newTaskId);

    /**
     * Joining the task to a task group. The method is called by the group on calling join with a reference
     * to the task instance. The method should never use by an application software.
     *
     * @param p_group Reference to the task group.
     *
     * @result Reference to the implementation part of the task.
     * @see Group::join
     */
    TaskImpl& joinTo(GroupImpl& p_group);

protected:
    /**
     * Second initialization step of construction using the input array from outside the class task. The method is
     * called by the constructor of the specialized class to connect the task with the inputs. If the template class
     * TaskProvider is used, which is the preferred way to set up a task, will call this method in the constructor.
     */
    void construct(void);

    /**
     * Pure virtual entrance point for the processing of the task. An implementation of a task should override this
     * method with the task specific processing.
     */
    virtual void execute(void) = 0;

    /**
     * Initialize the task. This step is performed by calling the initialize method of the associated scheduler.
     * The method can override by the application programmer with further initialization steps.
     *
     * @see Scheduler::initialize
     */
    virtual void initialize(void);

    /**
     * Request the associated channel pointer connected to an input. This call simplify the cast to the corresponding
     * channel type.
     *
     * @tparam channelType Expected type of the channel.
     *
     * @param key Key to identify the input to request the channel.
     *
     * @return Pointer to the associated channel at input with the key or null pointer if input is not connected to any
     *           channel.
     */
    template<typename channelType>
    channelType* getChannel(unsigned int key) const;

private:
    /// Forbid copy constructor
    Task(Task&);

    /// Implementation specific structure of task
    TaskImpl impl;
};

/**
 * Helper template to simplify set up of a task.
 *
 * @tparam numberOfInputs Number of inputs for the task
 *
 * @tparam Policy Scheduling policy type
 */
template<unsigned int numberOfInputs, class Policy>
class TaskProvider : public Task
{
public:
    /**
     * Constructor for a task with identification number
     *
     * @param scheduler Reference to the scheduler which performs this task.
     *
     * @param taskId Specify the ID number for a specific task.
     *
     * NOTE:
     *    It is the responsibility of the user to ensure uniqueness of the task id.
     * @see taskId_t
     */
    TaskProvider(Scheduler& scheduler, TaskId taskId = 0u);

    /**
     * Constructor for a task with identification number and a scheduling policy with settings.
     *
     * @param scheduler Reference to the scheduler which performs this task.
     *
     * @param settings Initial settings on the task for the scheduling policy.
     *
     * @param taskId Specify the identification for a specific task.
     *
     * NOTE:
     *    It is the responsibility of the user to ensure uniqueness of the task id.
     * @see taskId_t
     */
    TaskProvider(Scheduler& scheduler, typename Policy::Settings settings, TaskId taskId = 0u);

    /**
     * Constructor for a task with a name.
     *
     * @param scheduler Reference to the scheduler which performs this task.
     *
     * @param taskName Null-terminated string specifying a name for this task. The name will be
     *                    truncated after 4 characters. Only a name _or_ a taskId can be used for
     *                    channel identification.
     *
     * NOTE:
     *    It is the responsibility of the user to ensure uniqueness of the task id.
     * @see taskId_t
     */
    TaskProvider(Scheduler& scheduler, const char* taskName);

    /**
     * Constructor for a task with a name.
     *
     * @param scheduler Reference to the scheduler which performs this task.
     *
     * @param settings Initial settings on the task for the scheduling policy.
     *
     * @param taskName Null-terminated string specifying a name for this task. The name will be
     *                    truncated after 4 characters. Only a name _or_ a taskId can be used for
     *                    channel identification.
     */
    TaskProvider(Scheduler& scheduler, typename Policy::Settings settings, const char* taskName);

protected:
    /// Inputs of the task
    InputArrayProvider<numberOfInputs> inputs;

    /// Policy data of the task.
    typename Policy::ManagementData policyData; // Typename is needed to see the management data of the specified policy
};

// ========= implementation part ==========

template<typename channelType>
channelType*
Task::getChannel(unsigned int key) const
{
    return impl.inputs[key].getChannel<channelType>();
}

// ----------------

template<unsigned int numberOfInputs, class policy>
TaskProvider<numberOfInputs, policy>::TaskProvider(Scheduler& _scheduler, TaskId taskId) :
    Task(_scheduler, policyData, inputs, taskId)
{
    Task::construct();
}

template<unsigned int numberOfInputs, class policy>
TaskProvider<numberOfInputs, policy>::TaskProvider(Scheduler& _scheduler, typename policy::Settings settings,
                                                   TaskId taskId) :
    Task(_scheduler, policyData, inputs, taskId),
    policyData(settings)
{
    Task::construct();
}

template<unsigned int numberOfInputs, class policy>
TaskProvider<numberOfInputs, policy>::TaskProvider(Scheduler& _scheduler, const char* taskName) :
    TaskProvider(_scheduler, getTaskIdFromName(taskName))
{
}

template<unsigned int numberOfInputs, class policy>
TaskProvider<numberOfInputs, policy>::TaskProvider(Scheduler& _scheduler, typename policy::Settings settings,
                                                   const char* taskName) :
    TaskProvider(_scheduler, settings, getTaskIdFromName(taskName))
{
}

} // namespace Tasking

#endif /* TASK_H_ */
