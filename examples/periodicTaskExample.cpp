/*
 * periodicTaskExample.cpp
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

/*
 * This example is a simple example composed of one trigger and a task which generate output. The example can
 * configured to a relative or timing behavior.
 */

#include <iostream>
#include <string>

#include <schedulerProvider.h>
#include <schedulePolicyLifo.h>
#include <taskEvent.h>
#include <task.h>

/**
 * Define a task with one input which print out the current time of the clock. The task shall executed with the
 * schedule policy LIFO (Last In - First Out).
 */
class PeriodicTask : public Tasking::TaskProvider<1, Tasking::SchedulePolicyLifo>
{
public:
    /** Initialize the task
     * @param scheduler Reference to a scheduler with scheduling policy LIFO
     */
    PeriodicTask(Tasking::Scheduler& scheduler);

    /**
     * Overloading of the pure virtual method of a task, which implement the functionality of the task. For this
     * example it print out the task ID and the current time from the scheduler.
     */
    virtual void execute(void);

    /// Reference to the scheduler to request time
    Tasking::Scheduler& scheduler;
};

// Implement the functionality of the example task
PeriodicTask::PeriodicTask(Tasking::Scheduler& scheduler_) :
    TaskProvider(scheduler_, "PeriodicTask"), // Name will be truncated to 'Peri' because it only use 4 characters
    scheduler(scheduler_) // Get the reference to the clock of the scheduler to access them
{
    // Configure the one input of the task to expect one push on related channel for the activation of the task
    inputs[0].configure(1u, false);
}

void
PeriodicTask::execute(void)
{
    // Get the time from the clock of the scheduler This time start with 0 when the scheduler is started.
    Tasking::Time msAfterStart = scheduler.getTime();
    // Get the task identification of this task
    Tasking::TaskId myId = getTaskId();
    // The task id was calculated from the given task name, truncated to four characters, but it can read out as text
    char convertionArray[5];
    Tasking::convertChannelIdToString(myId, convertionArray, 5);
    std::string myTaskName(convertionArray);

    // Print out the information.
    std::cout << "Task Id " << myId << " (" << myTaskName << "): ";
    std::cout << "Current time after start is " << msAfterStart << "ms" << std::endl;
}

// ===== Instantiation of the example =====

/// The scheduler with LIFO scheduling policy
Tasking::SchedulerProvider<1u, Tasking::SchedulePolicyLifo> scheduler;

/// The trigger to start the execution of the example task
Tasking::Event trigger(scheduler);

/// The example task which is started by the trigger.
PeriodicTask task(scheduler);

// ===== main =====

int
main(void)
{
    // Connect the task with the trigger event at channel 0
    task.configureInput(0, trigger);

    // Configure the timing of the trigger, here a relative trigger is used. Use as trigger value 1 s.
    std::string keyBoardinput;
    std::cout << "Select timing: 'r' for relative timing, else periodic timing" << std::endl;
    std::cin >> keyBoardinput;
    if (keyBoardinput[0] == 'r')
    {
        trigger.setRelativeTiming(1000u);
    }
    else
    {
        trigger.setPeriodicTiming(500, 1000u);
    }

    // Start the scheduler with a reset action, which is needed to start relative timer
    scheduler.start(true);

    std::cout << "Type a line to terminate program" << std::endl;
    std::cin >> keyBoardinput;

    // Stop the relative trigger, its safer but not really necessary for the termination
    trigger.stop();

    // Terminate the scheduler
    scheduler.terminate();
}
