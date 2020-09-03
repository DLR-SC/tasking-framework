/*
 * testTaskInputArray.cpp
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

#include <taskInputArray.h>
#include <taskChannel.h>
#include <task.h>
#include <schedulerUnitTest.h>
#include <schedulePolicyLifo.h>

class TestTaskInputArray : public ::testing::Test
{
protected:
    class MChannel : public Tasking::Channel
    {
    public:
        MChannel(void) :
                        resetCalled(false)
        {
        }
        using Tasking::Channel::push;
        virtual void reset(void)
        {
            resetCalled = true;
        }
        bool resetCalled;
    };

    /// Receiving task which count the executions
    class CheckTask : public Tasking::Task
    {
    public:
        CheckTask(Tasking::Scheduler& scheduler) :
                        Task(scheduler, policy, inputs), implementation(scheduler, policy, *this, inputs)
        {
            // Nothing else to do.
        }
        /// Implement execute because it is necessary by default
        void execute(void)
        {
            // Nothing to do in this test
        }
        Tasking::InputArrayProvider<1u> inputs; // Not used in test
        Tasking::SchedulePolicyLifo::ManagementData policy; // Not used in test
        Tasking::TaskImpl implementation;
    };

    Tasking::SchedulePolicyLifo policy;
    Tasking::SchedulerUnitTest scheduler;
    MChannel channel;
    CheckTask task;

    TestTaskInputArray(void) :
                    scheduler(policy), task(scheduler)
    {
        task.reset();
    }
};

TEST_F(TestTaskInputArray, access)
{
    Tasking::InputArrayProvider<4u> inputs;

    EXPECT_EQ(4u, inputs.size());
}

TEST_F(TestTaskInputArray, isValid)
{
    Tasking::InputArrayProvider<3u> inputs;
    EXPECT_FALSE(inputs.isValid());
    inputs[0].configure(channel, 0u, true);
    EXPECT_FALSE(inputs.isValid());
    inputs[2].configure(channel, 4u);
    EXPECT_FALSE(inputs.isValid());
    inputs[1].configure(channel);
    EXPECT_FALSE(inputs.isValid()); // Not connected to task
    inputs.connectTask(task.implementation);
    EXPECT_TRUE(inputs.isValid());
}

TEST_F(TestTaskInputArray, isActivatedWithoutFinal)
{
    Tasking::InputArrayProvider<3u> inputs;
    MChannel channels[3];
    inputs.connectTask(task.implementation);
    inputs[0].configure(channels[0], 1u);
    inputs[1].configure(channels[1], 0u);
    inputs[2].configure(channels[2], 1u);
    EXPECT_FALSE(inputs.isActivated());
    channels[1].push();
    EXPECT_FALSE(inputs.isActivated());
    channels[2].push();
    EXPECT_FALSE(inputs.isActivated());
    channels[0].push();
    EXPECT_TRUE(inputs.isActivated());
}

TEST_F(TestTaskInputArray, isActivatedWithUntriggeredOptional)
{
    Tasking::InputArrayProvider<3u> inputs;
    MChannel channels[3];
    inputs.connectTask(task.implementation);
    inputs[0].configure(channels[0], 1u);
    inputs[1].configure(channels[1], 0u);
    inputs[2].configure(channels[2], 1u);
    EXPECT_FALSE(inputs.isActivated());
    channels[2].push();
    EXPECT_FALSE(inputs.isActivated());
    channels[0].push();
    EXPECT_TRUE(inputs.isActivated());
}

TEST_F(TestTaskInputArray, isActivatedWithFinal)
{
    Tasking::InputArrayProvider<3u> inputs;
    MChannel channels[3];
    inputs.connectTask(task.implementation);
    inputs[0].configure(channels[0], 1u);
    inputs[1].configure(channels[1], 0u, true);
    inputs[2].configure(channels[2], 1u);
    EXPECT_FALSE(inputs.isActivated());
    channels[2].push();
    EXPECT_FALSE(inputs.isActivated());
    channels[1].push();
    EXPECT_TRUE(inputs.isActivated());
}

TEST_F(TestTaskInputArray, isActivatedWithUntriggerOptionalFinal)
{
    Tasking::InputArrayProvider<3u> inputs;
    MChannel channels[3];
    inputs.connectTask(task.implementation);
    inputs[0].configure(channels[0], 1u);
    inputs[1].configure(channels[1], 0u, true);
    inputs[2].configure(channels[2], 1u);
    EXPECT_FALSE(inputs.isActivated());
    channels[2].push();
    EXPECT_FALSE(inputs.isActivated());
    channels[0].push();
    EXPECT_TRUE(inputs.isActivated());
}

TEST_F(TestTaskInputArray, reset)
{
    Tasking::InputArrayProvider<2u> inputs;
    MChannel channels[2];
    inputs[0].configure(channels[0], 1u);
    inputs[1].configure(channels[1]);
    inputs.reset();
    EXPECT_TRUE(channels[0].resetCalled);
    EXPECT_TRUE(channels[1].resetCalled);
}

