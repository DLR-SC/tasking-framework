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
        MChannel(void) : resetCalled(false), state(0)
        {
        }
        using Tasking::Channel::push;
        virtual void
        reset(void)
        {
            resetCalled = true;
        }
        bool resetCalled;
        int state;
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
        void
        execute(void)
        {
            // Nothing to do in this test
        }
        Tasking::InputArrayProvider<1u> inputs; // Not used in test
        Tasking::SchedulePolicyLifo::ManagementData policy; // Not used in test
        Tasking::TaskImpl implementation;
    };

    static bool conditionFalse(const Tasking::InputArray&);
    static bool conditionTrue(const Tasking::InputArray&);
    static bool conditionWithAccess(const Tasking::InputArray&);
    static bool stateCondition(const Tasking::InputArray&);

    Tasking::SchedulePolicyLifo policy;
    Tasking::SchedulerUnitTest scheduler;
    MChannel channel;
    CheckTask task;

    TestTaskInputArray(void) : scheduler(policy), task(scheduler)
    {
        task.reset();
    }
};

bool
TestTaskInputArray::conditionFalse(const Tasking::InputArray&)
{
    return false;
}

bool
TestTaskInputArray::conditionTrue(const Tasking::InputArray&)
{
    return true;
}

bool
TestTaskInputArray::conditionWithAccess(const Tasking::InputArray& array)
{
    if ((array.size()) > 1u && array[1u].isActivated())
    {
        return true;
    }
    return false;
}

bool
TestTaskInputArray::stateCondition(const Tasking::InputArray& array)
{
    const Tasking::Input& in = array[0u];
    return (in.isActivated() && in.getChannel<TestTaskInputArray::MChannel>()->state);
}

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

TEST_F(TestTaskInputArray, alternativeFunctionFalse)
{
    // Configure input array with two inputs expecting one activation and alternative false condition
    // The resulting input array will always get activated.
    Tasking::InputArrayProvider<2u> inputs;
    MChannel channels[2];
    inputs.setCondition(conditionFalse);
    inputs[0].configure(channels[0], 1u);
    inputs[1].configure(channels[1], 1u);
    inputs.connectTask(task.implementation);

    EXPECT_FALSE(inputs.isActivated());
    channels[0].push();
    EXPECT_FALSE(inputs.isActivated());
    channels[1].push();
    EXPECT_FALSE(inputs.isActivated());
}

TEST_F(TestTaskInputArray, alternativeFunctionTrue)
{
    // Configure input array with two inputs expecting one activation and alternative true condition
    // The resulting input array will get always activated.
    Tasking::InputArrayProvider<2u> inputs;
    MChannel channels[2];
    inputs.setCondition(conditionTrue);
    inputs[0].configure(channels[0], 1u);
    inputs[1].configure(channels[1], 1u);
    inputs.connectTask(task.implementation);

    EXPECT_TRUE(inputs.isActivated());
    channels[0].push();
    EXPECT_TRUE(inputs.isActivated());
    channels[1].push();
    EXPECT_TRUE(inputs.isActivated());
}

TEST_F(TestTaskInputArray, alternativeFunctionFalseFinal)
{
    // Configure input array with three inputs expecting one activation and alternative false condition.
    // Second one will be a final input.
    // The resulting input array will activated with channel 0 and 2 activated or if channel 1 is activated.
    Tasking::InputArrayProvider<3u> inputs;
    MChannel channels[3];
    inputs.setCondition(conditionFalse);
    inputs[0].configure(channels[0], 1u);
    inputs[1].configure(channels[1], 1u, true);
    inputs[2].configure(channels[2], 1u);
    inputs.connectTask(task.implementation);

    EXPECT_FALSE(inputs.isActivated());
    channels[0].push();
    EXPECT_FALSE(inputs.isActivated());
    channels[1].push();
    EXPECT_TRUE(inputs.isActivated());
    channels[2].push();
    EXPECT_TRUE(inputs.isActivated());
}

TEST_F(TestTaskInputArray, alternativeFunctionWithAccess)
{
    // Configure input array with two inputs expecting one activation with input array access.
    // The resulting input array will get activated it the second input is pushed.
    Tasking::InputArrayProvider<2u> inputs;
    MChannel channels[2];
    inputs.setCondition(conditionWithAccess);
    inputs[0].configure(channels[0], 1u);
    inputs[1].configure(channels[1], 1u);
    inputs.connectTask(task.implementation);

    EXPECT_FALSE(inputs.isActivated());
    channels[0].push();
    EXPECT_FALSE(inputs.isActivated());
    inputs.reset();
    channels[1].push();
    EXPECT_TRUE(inputs.isActivated());
}

TEST_F(TestTaskInputArray, alternativeFunctionTriggerStateMachinePath)
{
    // Configure input array with one input expecting an activation depending on the state in the input channel
    // Such a behavior can use to trigger a path of tasks depending on a current state.
    // The resulting input array will get activated if the input has state 1.
    Tasking::InputArrayProvider<1u> inputs;
    MChannel testChannel;
    inputs.setCondition(stateCondition);
    inputs[0].configure(testChannel, 1u);
    inputs.connectTask(task.implementation);

    EXPECT_FALSE(inputs.isActivated());
    testChannel.push();
    EXPECT_FALSE(inputs.isActivated());
    inputs.reset();
    testChannel.state = 1;
    testChannel.push();
    EXPECT_TRUE(inputs.isActivated());
}
