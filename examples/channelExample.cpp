/*
 * ChannelExample.cpp
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
 * This example will compute Fibonacci numbers with the Tasking Framework.
 * The example use two tasks in a group, one for the calculation of the fibonacci number and one for printing.
 * The execution order of both tasks is satisfy by priorities.
 * To speed down a trigger is used on the calculation. Removing this input will run the program faster.
 */

#include <iostream>
#include <unistd.h> // For usleep

#include <schedulerProvider.h>
#include <schedulePolicyPriority.h>
#include <taskGroup.h>
#include <taskChannel.h>
#include <taskInput.h>
#include <task.h>

/**
 * A channels which provide the last numbers pushed on it.
 */
class FibonacciMemory : public Tasking::Channel
{
public:
    /**
     * Setting up the channel
     */
    FibonacciMemory();

    /**
     * Insert a new element. The oldest one is removed from the channel
     * @param data New element becoming first element on the queue
     */
    void pushFibo(unsigned long long data);

    /**
     * @param age Index to the data in the channel. Value zero is the last pushed to the shifting channel.
     * @return Value with the given age, or zero if out of index
     */
    unsigned long long get(unsigned int age) const;

    /// @return True when an overflow was detected
    bool isOverflow(void) const;

private:
    /// Memory to hold the Fibonaccy number
    unsigned long long array[2u];
    /// Index to newest element in the array
    unsigned int newest;
    /// Flag to indicate an overflow
    bool overflow;
};

FibonacciMemory::FibonacciMemory() : newest(0), overflow(false)
{
    for (unsigned int i = 0u; i < 2u; ++i)
    {
        array[i] = 0u;
    }
}

void
FibonacciMemory::pushFibo(unsigned long long data)
{
    // Check on overflow computation is always + operation
    if (data < array[newest])
    {
        overflow = true;
        // No push in case of the overflow will stop the computation
    }
    else
    {
        // Stop run when an overflow happen
        newest = (newest + 1u) % 2u;
        array[newest] = data;
        Channel::push();
    }
}

unsigned long long
FibonacciMemory::get(unsigned int age) const
{
    unsigned long long result = 0u;
    if (age < 2)
    {
        result = array[(newest - age) % 2u];
    }
    return result;
}

bool
FibonacciMemory::isOverflow() const
{
    return overflow;
}

/**
 * A task which compute a fibonacci number. It take the last two number from the incoming channel.
 * The result is pushed to the same channel.
 */
class FibonacciTask : public Tasking::TaskProvider<2u, Tasking::SchedulePolicyPriority>
{
public:
    /**
     * Initialize task.
     * @param scheduler Reference to the scheduler which execute the task.
     */
    FibonacciTask(Tasking::Scheduler& scheduler);

    /// Computation of the next Fibonacci number
    virtual void execute(void);
};

FibonacciTask::FibonacciTask(Tasking::Scheduler& scheduler) :
    TaskProvider(scheduler, Tasking::SchedulePolicyPriority::Settings(1u))
{
    inputs[0].configure(1u); // Run when one data item is pushed
    inputs[0].setSynchron(); // The own push on the channel should activate the task again.
    inputs[1].configure(1u);
}

void
FibonacciTask::execute(void)
{
    FibonacciMemory& channel = *getChannel<FibonacciMemory>(0u);
    unsigned long long fibonaccy = channel.get(0) + channel.get(1);
    channel.pushFibo(fibonaccy);
}

/**
 * A task which print out the last number from a channel.
 */

class PrinterTask : public Tasking::TaskProvider<1u, Tasking::SchedulePolicyPriority>
{
public:
    /**
     * Initialize task.
     * @param scheduler Reference to the scheduler which execute the task.
     */
    PrinterTask(Tasking::Scheduler& scheduler);

    /// Print out data from the channel
    virtual void execute(void);
};

PrinterTask::PrinterTask(Tasking::Scheduler& scheduler) :
    TaskProvider(scheduler, Tasking::SchedulePolicyPriority::Settings(2u))
// Priority is higher than computation of Fibonaccy number, so it print the result from the last computation cycle.
// When priorities are otherwise printing of first numbers are not correctly
{
    inputs[0].configure(1u);
    inputs[0].setSynchron(); // For each element one call and push will happen before group is reset
}

void
PrinterTask::execute(void)
{
    std::cout << getChannel<FibonacciMemory>(0u)->get(0) << std::endl;
}

// ===== Setup system =====

/// The scheduler with priority scheduling policy
Tasking::SchedulerProvider<1u, Tasking::SchedulePolicyPriority> scheduler;

/// Channel with the Fibonaccy numbers
FibonacciMemory fibonaccyNumbers;

/// Event to reduce the speed to a relative timing
Tasking::Event trigger(scheduler);

/// Task to compute Fibonaccy numbers
FibonacciTask fibonaccyTask(scheduler);

/// Task to print out Fibonaccy numbers
PrinterTask printerTask(scheduler);

Tasking::GroupProvider<2u> group;

// ===== main =====

int
main(void)
{
    // Connect the tasks with the fibonaccy memory channel at input 0
    printerTask.configureInput(0u, fibonaccyNumbers);

    // Start the scheduler with a reset action
    scheduler.start(true);

    std::cout << "The program terminate when an overflow is detected" << std::endl;

    // Shift in the first to number of the Fibonaccy row
    fibonaccyNumbers.pushFibo(0u);
    // Wait until printer task is executed one time
    scheduler.terminate(true);

    // Add Fibonaccy task now to the channel and group both tasks, so execution times are equal from now on
    fibonaccyTask.configureInput(0, fibonaccyNumbers);
    fibonaccyTask.configureInput(1, trigger);
    trigger.setRelativeTiming(200u); // Limit computation to only every 200 ms
    group.join(printerTask); // The example works also without the group by the priorities and speed limitation.
    group.join(fibonaccyTask);

    // Restart scheduler
    scheduler.start();
    fibonaccyNumbers.pushFibo(1u);

    while (!fibonaccyNumbers.isOverflow())
    {
        usleep(50);
    }

    // Terminate the scheduler
    scheduler.terminate();
}
