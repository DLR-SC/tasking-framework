/*
 * testTaskInput.cpp
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
#include <task.h>
#include <taskInputArray.h>
#include <taskChannel.h>
#include <schedulerUnitTest.h>
#include <schedulePolicyLifo.h>

class TestTaskInput : public ::testing::Test
{
public:

    class TestChannel : public Tasking::Channel
    {
    public:
        TestChannel(void)
        {
            // Nothing else to do.
        }
        using Tasking::Channel::push;
    };

    /// Receiving task which count the executions
    class CheckTask : public Tasking::Task
    {
    public:
        CheckTask(Tasking::Scheduler& scheduler) :
                        Task(scheduler, policyData, inputs), implementation(scheduler, policyData, *this, inputs)
        {
            // Nothing else to do.
        }
        /// Implement execute because it is necessary by default
        void execute(void)
        {
            // Nothing to do in this test
        }
        Tasking::InputArrayProvider<1u> inputs;
        Tasking::SchedulePolicyLifo::ManagementData policyData;
        Tasking::TaskImpl implementation;
    };



//    /// Instance of a task is always needed to prevent null pointer exception
    Tasking::SchedulePolicyLifo policy;
    Tasking::SchedulerUnitTest scheduler;
    CheckTask checker;
    TestChannel channel;
    Tasking::Input input;

    TestTaskInput(void) :
                    scheduler(policy), checker(scheduler)
    {
        input.connectTask(checker.implementation); // Not correct initialization, because checker has an own input. For these tests OK.
        input.configure(channel);
    }
};

TEST_F(TestTaskInput, getActivations)
{
    input.setSynchron(false);
    EXPECT_EQ(0u, input.getActivations());
    // input.notifyInput() is not accessable, but push on channel call it.
    channel.push();
    EXPECT_EQ(1u, input.getActivations());
    channel.push();
    EXPECT_EQ(2u, input.getActivations());
    input.reset();
    EXPECT_EQ(0u, input.getActivations());
}

TEST_F(TestTaskInput, isActivated)
{
    input.configure(channel, 2u);
    EXPECT_FALSE(input.isActivated());
    channel.push();
    EXPECT_FALSE(input.isActivated());
    channel.push();
    EXPECT_TRUE(input.isActivated());
    channel.push();
    EXPECT_TRUE(input.isActivated());
    input.reset();
    EXPECT_FALSE(input.isActivated());
}

TEST_F(TestTaskInput, synchonized) {
    input.configure(channel, 2u);
    channel.push();
    channel.push();
    channel.push();
    channel.push();
    channel.push();
    EXPECT_TRUE(input.isActivated());
    EXPECT_EQ(2u, input.getActivations());
    input.reset();
    EXPECT_TRUE(input.isActivated());
    EXPECT_EQ(2u, input.getActivations());
    input.reset();
    EXPECT_FALSE(input.isActivated());
    channel.push();
    EXPECT_TRUE(input.isActivated());
}

TEST_F(TestTaskInput, optionalBecomesNotSynchronized) {
    input.configure(0u);
    input.setSynchron(false);
    channel.push();
    channel.push();
    EXPECT_EQ(2u, input.getActivations());
    input.reset();
    EXPECT_EQ(0u, input.getActivations());
}

TEST_F(TestTaskInput, ConfigureOptionalRemoveSynchronized) {
    input.configure(0u);
    channel.push();
    channel.push();
    EXPECT_EQ(2u, input.getActivations());
    input.reset();
    EXPECT_EQ(0u, input.getActivations());
}

TEST_F(TestTaskInput, getChannel)
{
    input.configure(channel);
    TestChannel* requestedChannel = input.getChannel<TestChannel>();
    EXPECT_TRUE(requestedChannel == &channel);
}
