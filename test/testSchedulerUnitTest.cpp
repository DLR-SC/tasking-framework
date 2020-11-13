/*
 * testSchedulerUnitTest.cpp
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

#include <gtest/gtest.h>

#include <schedulerUnitTest.h>
#include <task.h>
#include <taskInput.h>
#include <taskChannel.h>
#include <taskEvent.h>
#include <schedulerUnitTest.h>
#include <schedulePolicyLifo.h>

/**
 * Checking the functionality of the scheduler for unit tests. It also check methods of the scheduler which are common
 * for all scheduler implementations.
 */
class TestSchedulerUnitTest : public ::testing::Test
{
public:

    TestSchedulerUnitTest(void) :
                    scheduler(policy), checker(scheduler)
    {
        // input 1 is not used in every test case, don't add it here. Add it in test case
        checker.configureInput(0, msg[0]);
        checker.configureInput(1, msg[1]);
    }

    ~TestSchedulerUnitTest(void)
    {
    }

    class CheckChannel : public Tasking::Channel
    {
    public:
        CheckChannel(void) :
                        Channel(1), resets(0), startsynchTask(NULL), synchActivations(0), endsynchTask(NULL)
        {
            // Nothing else to do.
        }

        using Tasking::Channel::push;

        virtual void reset(void)
        {
            ++resets;
        }
        void synchronizeStart(const Tasking::Task* task, unsigned int activations) override
        {
            startsynchTask = task;
            synchActivations = activations;
        }
        virtual void synchronizeEnd(Tasking::Task* task)
        {
            endsynchTask = task;
        }
        int resets;
        const Tasking::Task* startsynchTask;
        int synchActivations;
        Tasking::Task* endsynchTask;
    };

    /// Receiving task which count the executions. The task hold two inputs with 1 activation until start
    class CheckTask : public Tasking::TaskProvider<2u, Tasking::SchedulePolicyLifo>
    {
    public:
        CheckTask(Tasking::Scheduler& scheduler, const Tasking::TaskId id = 0u) :
                        TaskProvider(scheduler, id), calls(0), resets(0), initializations(0)
        {
            // Configure input settings
            inputs[0].configure(1u);
            inputs[1].configure(1u);
        }

        /// Overload execute to count executions
        virtual void execute(void)
        {
            calls++;
        }
        /// Count reset calls
        virtual void reset(void)
        {
            resets++;
            Task::reset();
        }
        /// Count reset operations
        virtual void initialize(void)
        {
            initializations++;
            Task::initialize();
        }
        /// Provide inputs as public.
        using Tasking::TaskProvider<2u, Tasking::SchedulePolicyLifo>::inputs;
        /// Provide getChannel as public.
        using Tasking::TaskProvider<2u, Tasking::SchedulePolicyLifo>::getChannel;
        /// Number of executions
        int calls;
        /// Number of reset operations
        int resets;
        /// Number of initializations
        int initializations;
    };

    /// The used policy for the tests
    Tasking::SchedulePolicyLifo policy;
    /// Scheduler under test
    Tasking::SchedulerUnitTest scheduler;
    /// Task definition for unit tests
    CheckTask checker;
    /// Two messages to connect with the task.
    CheckChannel msg[2];
};

TEST_F(TestSchedulerUnitTest, noTaskToPerform)
{
    scheduler.start(false);
    scheduler.schedule();
    EXPECT_EQ(0, checker.calls);
    EXPECT_EQ(0, msg[0].resets);
    EXPECT_EQ(0, msg[0].resets);
}

TEST_F(TestSchedulerUnitTest, initialize)
{
    scheduler.initialize();
    EXPECT_EQ(1, checker.initializations);
    CheckTask furtherTask(scheduler);
    scheduler.initialize();
    EXPECT_EQ(2, checker.initializations);
    EXPECT_EQ(1, furtherTask.initializations);
}

TEST_F(TestSchedulerUnitTest, reset)
{
    scheduler.start();
    EXPECT_EQ(1, msg[0].resets);
    EXPECT_EQ(1, msg[0].resets);
    EXPECT_EQ(1, checker.resets);
    CheckTask furtherTask(scheduler);
    scheduler.start();
    EXPECT_EQ(2, msg[0].resets);
    EXPECT_EQ(2, msg[0].resets);
    EXPECT_EQ(2, checker.resets);
    EXPECT_EQ(1, furtherTask.resets);
}

TEST_F(TestSchedulerUnitTest, performTask)
{
    scheduler.start();
    msg[0].push();
    msg[1].push();
    scheduler.schedule();
    EXPECT_EQ(1, checker.calls);
    EXPECT_EQ(2, msg[0].resets);
    EXPECT_EQ(2, msg[0].resets);
    EXPECT_TRUE(msg[0].startsynchTask == &checker);
    EXPECT_EQ(1, msg[0].synchActivations);
    EXPECT_TRUE(msg[0].endsynchTask == &checker);
}

TEST_F(TestSchedulerUnitTest, performSeveralTasks)
{
    CheckTask furtherTask(scheduler);
    CheckChannel fmsg[2];
    furtherTask.configureInput(0u, fmsg[0]);
    furtherTask.configureInput(1u, fmsg[1]);
    scheduler.start();
    msg[0].push();
    msg[1].push();
    fmsg[0].push();
    fmsg[1].push();
    scheduler.schedule();
    EXPECT_EQ(1, checker.calls);
    EXPECT_EQ(2, msg[0].resets);
    EXPECT_EQ(2, msg[0].resets);
    EXPECT_EQ(1, furtherTask.calls);
    EXPECT_EQ(2, fmsg[0].resets);
    EXPECT_EQ(2, fmsg[0].resets);
}

TEST_F(TestSchedulerUnitTest, terminateBeforeActivation)
{
    scheduler.start();
    msg[0].push();
    scheduler.terminate();
    msg[1].push();
    scheduler.schedule();
    EXPECT_EQ(0, checker.calls);
    EXPECT_EQ(1, msg[0].resets);
    EXPECT_EQ(1, msg[0].resets);
}

TEST_F(TestSchedulerUnitTest, terminateAfterActivation)
{
    scheduler.start();
    msg[0].push();
    msg[1].push();
    scheduler.terminate();
    scheduler.schedule();
    EXPECT_EQ(0, checker.calls);
    EXPECT_EQ(1, msg[0].resets);
    EXPECT_EQ(1, msg[0].resets);
    // Without reset option start should check the activation state of tasks
    scheduler.start(false);
    EXPECT_EQ(0, checker.calls);
    EXPECT_EQ(1, msg[0].resets);
    EXPECT_EQ(1, msg[0].resets);
    scheduler.schedule();
    EXPECT_EQ(1, checker.calls);
    EXPECT_EQ(2, msg[0].resets);
    EXPECT_EQ(2, msg[0].resets);
    // Check again with default reset option at start
    msg[0].push();
    msg[1].push();
    scheduler.terminate();
    scheduler.start(true);
    scheduler.schedule();
    EXPECT_EQ(1, checker.calls);
    EXPECT_EQ(3, msg[0].resets);
    EXPECT_EQ(3, msg[0].resets);
}

TEST_F(TestSchedulerUnitTest, terminateWithoutCleanUp)
{
    scheduler.start();
    msg[0].push();
    msg[1].push();
    scheduler.terminate(true);
    scheduler.schedule();
    EXPECT_EQ(1, checker.calls);
    EXPECT_EQ(2, msg[0].resets);
    EXPECT_EQ(2, msg[0].resets);
}

TEST_F(TestSchedulerUnitTest, handleEvent)
{
    Tasking::Event event(scheduler);
    CheckChannel channel;
    CheckTask timeTriggeredTask(scheduler);
    timeTriggeredTask.configureInput(0u, event);
    timeTriggeredTask.configureInput(1u, channel);
    scheduler.start();
    channel.push();
    event.trigger(); // Both inputs are activated now
    scheduler.schedule();
    EXPECT_EQ(1, timeTriggeredTask.calls);
}

TEST_F(TestSchedulerUnitTest, stepOverTime)
{
    Tasking::Event event(scheduler);
    CheckChannel channel;
    CheckTask timeTriggeredTask(scheduler);
    timeTriggeredTask.configureInput(0u, event);
    timeTriggeredTask.configureInput(1u, channel);
    scheduler.start();
    channel.push();
    event.trigger(1);
    scheduler.schedule();
    EXPECT_EQ(0, timeTriggeredTask.calls);
    // Step one ms ahead
    scheduler.schedule(1u);
    EXPECT_EQ(1, timeTriggeredTask.calls);
}
