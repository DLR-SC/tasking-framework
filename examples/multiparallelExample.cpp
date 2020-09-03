/*
 * multiparallelExample.cpp
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

/*
 * This is an example for the usage of a barrier. One event triggers ten tasks with 1000Hz. The task
 * prints a dot and notifies end of execution by a push to a barrier. The barrier is connected with a task
 * which prints out its name and identification.
 */

#include <iostream>
#include <string>

#include <schedulerProvider.h>
#include <schedulePolicyFifo.h>
#include <taskEvent.h>
#include <task.h>
#include <taskBarrier.h>

/**
 * Define a task with one input which print out the current time of the clock. The task shall executed with the
 * schedule policy FIFO (First In - First Out).
 */
class PrinterTask : public Tasking::TaskProvider<1, Tasking::SchedulePolicyFifo>
{
public:
    /** Initialize the task
     * @param scheduler Reference to a scheduler with scheduling policy LIFO
     */
    PrinterTask(Tasking::Scheduler& scheduler);

    /**
     * Overloading of the pure virtual method of a task, which implement the functionality of the task. For this
     * example it print out the task ID and the current time from the scheduler.
     */
    void execute(void) override;

    /// Reference to the scheduler to request time
    Tasking::Scheduler& scheduler;
};

// Implement the functionality of the example task
PrinterTask::PrinterTask(Tasking::Scheduler& scheduler_) :
    TaskProvider(scheduler_, "PeriodicTask"), // Name will be truncated to 'Peri' because it only use 4 characters
    scheduler(scheduler_) // Get the reference to the clock of the scheduler to access them
{
    // Configure the one input of the task to expect one arrival at the input for the activation of the task
    inputs[0].configure(1u, false);
}

void
PrinterTask::execute(void)
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

/// Task which will be executed in parallel
class ParallelTask : public Tasking::TaskProvider<1, Tasking::SchedulePolicyFifo>
{
public:
    ParallelTask(Tasking::Scheduler& scheduler, Tasking::Barrier& barrier) :
        TaskProvider(scheduler),
        outBarrier(barrier)
    {
        inputs[0].configure(1);
    }

    void
    execute(void) override
    {
        std::cout << "." << std::endl;
        outBarrier.push();
    }

private:
    Tasking::Barrier& outBarrier;
};

// ===== Instantiation of the example =====

// Configure the test.
static const unsigned int parallelity = 10;
static const bool periodicTrigger = false;
unsigned int time_ms = 1;

/// The scheduler with LIFO scheduling policy
Tasking::SchedulerProvider<1u, Tasking::SchedulePolicyFifo> scheduler;

/// The trigger to start the execution of the example task
Tasking::Event trigger(scheduler);

/// Barrier to combine parallel threads and start example task;
Tasking::Barrier barrier(parallelity);

/// Array to hold the parallel tasks. They must be initialized by new.
ParallelTask* parallelTasks[parallelity];

/// The example task which is started by the trigger.
PrinterTask exampleTask(scheduler);

// ===== main =====

int
main(void)
{
    // Generate parallel tasks
    for (unsigned int i = 0; i < parallelity; ++i)
    {
        parallelTasks[i] = new ParallelTask(scheduler, barrier);
        parallelTasks[i]->configureInput(0, trigger);
    }

    // Connect the example task with the barrier at channel 0. If all parallel tasks are executed the example task will
    // start.
    exampleTask.configureInput(0, barrier);

    scheduler.setZeroTime(0);
    if (periodicTrigger)
    {
        trigger.setPeriodicTiming(time_ms, 1000u);
    }
    else
    {
        trigger.setRelativeTiming(time_ms);
    }

    // Start the scheduler with a reset action, which is needed to start relative timer
    scheduler.start(true);

    std::cout << "Type a line to terminate program" << std::endl;
    std::string keyBoardInput;
    std::cin >> keyBoardInput;

    // Stop the relative trigger, its safer but not really necessary for the termination
    trigger.stop();

    // Terminate the scheduler
    scheduler.terminate();
}
