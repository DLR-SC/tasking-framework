/*
 * testSingleBuffer.cpp
 *
 * Copyright 2012-2020 German Aerospace Center (DLR) SC
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

#include "gtest/gtest.h"
#include <cstring>

#include <channels/singleBuffer.h>
#include <task.h>
#include <schedulerUnitTest.h>
#include <schedulePolicyLifo.h>

using Tasking::SingleBuffer;

class TestSingleBuffer : public ::testing::Test
{
public:
    class TestTask : public Tasking::TaskProvider<1, Tasking::SchedulePolicyLifo>
    {
    public:
        /// Configure task with 1 input with one necessary push on related channel and not final
        TestTask(Tasking::Scheduler& p_scheduler) : TaskProvider<1, Tasking::SchedulePolicyLifo>(p_scheduler)
        {
            inputs[0].configure(1, false);
        }
        void
        execute(void) override
        {
        }
        /// Provide access to inputs for testing
        using TaskProvider::inputs;
    };

    TestSingleBuffer(void) : scheduler(policy), task(scheduler)
    {
        task.configureInput(0, channel);
    }

    ~TestSingleBuffer(void)
    {
        std::memset(static_cast<void*>(&channel), 1, sizeof(channel)); // Invalidate memory for next test
    }

protected:
    Tasking::SchedulePolicyLifo policy;
    Tasking::SchedulerUnitTest scheduler;
    TestTask task;
    SingleBuffer<int> channel;
};

TEST_F(TestSingleBuffer, ConstructionWithId)
{
    // In this case the integer literal is important!!
    // With integer literal channelId becomes 0 and the integer value is taken as initializer.
    SingleBuffer<int> constructedChannel(1726u);
    EXPECT_EQ(1726u, constructedChannel.getChannelId());
}

TEST_F(TestSingleBuffer, ConstructionWithName)
{
    SingleBuffer<int> constructedChannel("HoHo");
    EXPECT_EQ(0x486F486Fu, constructedChannel.getChannelId());
}

TEST_F(TestSingleBuffer, assignmentConstruction)
{
    SingleBuffer<int> constructedChannel(42, "HoHo");
    EXPECT_EQ(42, constructedChannel.read());
}

TEST_F(TestSingleBuffer, send)
{
    channel.send(42);
    EXPECT_EQ(42, channel.read());
    EXPECT_TRUE(task.inputs[0].isActivated());
}

TEST_F(TestSingleBuffer, sendPointer)
{
    int value = 42;
    channel.send(&value);
    EXPECT_EQ(42, channel.read());
    EXPECT_TRUE(task.inputs[0].isActivated());
}

TEST_F(TestSingleBuffer, allocateSend)
{
    channel.send(42);
    int* pointer = channel.getBuffer();
    EXPECT_EQ(42, *pointer);
    task.reset();
    *pointer = 24;
    channel.send(pointer);
    EXPECT_EQ(24, channel.read());
    EXPECT_TRUE(task.inputs[0].isActivated());
}
