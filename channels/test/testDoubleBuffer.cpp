/*
 * testDoubleBuffer.cpp
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

#include <channels/doubleBuffer.h>
#include <task.h>
#include <schedulerUnitTest.h>
#include <schedulePolicyLifo.h>

using Tasking::DoubleBuffer;

class TestDoubleBuffer : public ::testing::Test
{
public:
    class TestTask : public Tasking::TaskProvider<1, Tasking::SchedulePolicyLifo>
    {
    public:
        /// Configure task with 1 input with one necessary push on related channel and not final
        TestTask(Tasking::Scheduler& p_scheduler) :
            TaskProvider<1, Tasking::SchedulePolicyLifo>(p_scheduler), readValue(0)
        {
            inputs[0].configure(1, false);
        }
        void
        execute(void) override
        {
            readValue = getChannel<DoubleBuffer<int>>(0u)->read();
        }
        /// Provide access to inputs for testing
        using TaskProvider::inputs;
        /// By the task read value.
        int readValue;
    };

    TestDoubleBuffer(void) : scheduler(policy), task(scheduler)
    {
        task.configureInput(0, channel);
        scheduler.start();
    }

    ~TestDoubleBuffer(void)
    {
        std::memset(static_cast<void*>(&channel), 1, sizeof(channel)); // Invalidate memory for next test
    }

protected:
    Tasking::SchedulePolicyLifo policy;
    Tasking::SchedulerUnitTest scheduler;
    TestTask task;
    DoubleBuffer<int> channel;
};

TEST_F(TestDoubleBuffer, constructionWithId)
{
    // In this case the integer literal is important!!
    // With integer literal channelId becomes 0 and the integer value is taken as initializer.
    DoubleBuffer<int> constructedChannel(1726u);
    EXPECT_EQ(1726u, constructedChannel.getChannelId());
}

TEST_F(TestDoubleBuffer, constructionWithName)
{
    DoubleBuffer<int> constructedChannel("HoHo");
    EXPECT_EQ(0x486F486Fu, constructedChannel.getChannelId());
}

TEST_F(TestDoubleBuffer, initializedConstruction)
{
    DoubleBuffer<int> constructedChannel(42);
    EXPECT_TRUE(true);
    EXPECT_EQ(42, constructedChannel.read());
    EXPECT_EQ(42, *(constructedChannel.getBuffer()));
}

TEST_F(TestDoubleBuffer, initializedConstructionWithName)
{
    DoubleBuffer<int> constructedChannel(47, "HoHo");
    EXPECT_EQ(47, constructedChannel.read());
    EXPECT_EQ(47, *(constructedChannel.getBuffer()));
    EXPECT_EQ(0x486F486Fu, constructedChannel.getChannelId());
}

TEST_F(TestDoubleBuffer, sendValue)
{
    channel.send(42);
    ASSERT_TRUE(task.inputs[0].isActivated());
    EXPECT_EQ(42, (channel.read()));
}

TEST_F(TestDoubleBuffer, divergentPointerBetweenExecutions)
{
    const int* p0 = &(channel.read());
    channel.send(42);
    scheduler.schedule();
    const int* p1 = &(channel.read());
    EXPECT_TRUE((p0 != p1));
    // Switch over the two elements
    channel.send(42);
    p1 = &(channel.read());
    EXPECT_TRUE((p0 == p1));
}

TEST_F(TestDoubleBuffer, divergentPointerBetweenReadAndAllocated)
{
    const int* p0 = &(channel.read());
    const int* p1 = channel.getBuffer();
    EXPECT_TRUE((p0 != p1));
    // After send the back buffer is the pointer to the previous read data element
    channel.send(0);
    EXPECT_TRUE((p0 == channel.getBuffer()));
}

TEST_F(TestDoubleBuffer, sendPointer)
{
    int value = 42;
    channel.send(&value);
    EXPECT_EQ(42, channel.read());
    EXPECT_TRUE(task.inputs[0].isActivated());
}

TEST_F(TestDoubleBuffer, allocateSendPointer)
{
    int* pointer = channel.getBuffer();
    EXPECT_FALSE(task.inputs[0].isActivated());
    *pointer = 24;
    channel.send(pointer);
    EXPECT_EQ(24, channel.read());
    EXPECT_TRUE(task.inputs[0].isActivated());
}

TEST_F(TestDoubleBuffer, allocateSend)
{
    int value = 42;
    int* pointer = channel.getBuffer();
    *pointer = 24;
    channel.send(&value);
    EXPECT_EQ(42, channel.read());
    // Send shall write on the back buffer
    EXPECT_EQ(42, *pointer);
    EXPECT_TRUE(task.inputs[0].isActivated());
}
