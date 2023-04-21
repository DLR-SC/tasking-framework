/*
 * testTaskChannel.cpp
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
#include <taskInput.h>
#include <taskChannel.h>

class TestTaskChannel : public ::testing::Test
{
public:
    // Class for direct access to protected methods
    class IDChannel : public Tasking::Channel
    {
    public:
        IDChannel(Tasking::ChannelId id = 0) : Tasking::Channel(id)
        {
            // Nothing else to do.
        }
        IDChannel(const char* name) : Tasking::Channel(name)
        {
            // Nothing else to do.
        }
        using Tasking::Channel::associateTo;
        using Tasking::Channel::deassociate;
        using Tasking::Channel::push;
    };
};

TEST_F(TestTaskChannel, testTaskChannelId)
{
    IDChannel channel(78u);
    EXPECT_EQ(78u, channel.getChannelId());
    channel.setChannelId(65u);
    EXPECT_EQ(65u, channel.getChannelId());
}

TEST_F(TestTaskChannel, testTaskChannelName)
{
    IDChannel channel("5,-$");
    EXPECT_EQ(0x352C2D24u, channel.getChannelId());
    channel.setChannelName("foo");
    EXPECT_EQ(0x666F6F00u, channel.getChannelId());
    IDChannel longNameChannel("LongName");
    EXPECT_EQ(0x4C6F6E67u, longNameChannel.getChannelId());
}

TEST_F(TestTaskChannel, associateDeassociate)
{
    IDChannel channel;
    Tasking::Input inputs[6];
    // Deassociation on empty channel should not work
    inputs[0].deassociate();
    for (unsigned int i = 0u; i < 5u; ++i)
    {
        // Call of Channel::associateTo must be indirect over Input::associate
        inputs[i].associate(channel);
    }
    // Remove one input in the middle, call is only possible by indirect call of Input::deassociate instead of
    // Channel::deassociate
    inputs[2].deassociate();
    channel.push();
    EXPECT_EQ(1u, inputs[0].getNotifications());
    EXPECT_EQ(1u, inputs[1].getNotifications());
    EXPECT_EQ(0u, inputs[2].getNotifications());
    EXPECT_EQ(1u, inputs[3].getNotifications());
    EXPECT_EQ(1u, inputs[4].getNotifications());
    // Remove first associated
    inputs[0].deassociate();
    channel.push();
    EXPECT_EQ(1u, inputs[0].getNotifications());
    EXPECT_EQ(2u, inputs[1].getNotifications());
    EXPECT_EQ(0u, inputs[2].getNotifications());
    EXPECT_EQ(2u, inputs[3].getNotifications());
    EXPECT_EQ(2u, inputs[4].getNotifications());
    // Remove last associated
    inputs[4].deassociate();
    channel.push();
    EXPECT_EQ(1u, inputs[0].getNotifications());
    EXPECT_EQ(3u, inputs[1].getNotifications());
    EXPECT_EQ(0u, inputs[2].getNotifications());
    EXPECT_EQ(3u, inputs[3].getNotifications());
    EXPECT_EQ(2u, inputs[4].getNotifications());
    // Remove not associated
    inputs[5].deassociate();
    EXPECT_EQ(1u, inputs[0].getNotifications());
    EXPECT_EQ(3u, inputs[1].getNotifications());
    EXPECT_EQ(0u, inputs[2].getNotifications());
    EXPECT_EQ(3u, inputs[3].getNotifications());
    EXPECT_EQ(2u, inputs[4].getNotifications());
    // Remove all remaining
    inputs[1].deassociate();
    inputs[3].deassociate();
    channel.push();
    EXPECT_EQ(1u, inputs[0].getNotifications());
    EXPECT_EQ(3u, inputs[1].getNotifications());
    EXPECT_EQ(0u, inputs[2].getNotifications());
    EXPECT_EQ(3u, inputs[3].getNotifications());
    EXPECT_EQ(2u, inputs[4].getNotifications());
}

TEST_F(TestTaskChannel, testWrongfulAssociation)
{
    IDChannel channel;
    Tasking::Input input;
    EXPECT_TRUE(input.associate(channel));
    EXPECT_FALSE(input.associate(channel));
}
