/*
 * testTaskUtils.cpp
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

#include <gtest/gtest.h>
#include <string.h> // For memcmp

#include <taskUtils.h>
#include <task.h>
#include <taskChannel.h>
#include <schedulerUnitTest.h>
#include <schedulePolicyFifo.h>

class TestUtils : public ::testing::Test
{
public:
    // Provide class with execute method.
    class TestTask : public Tasking::TaskProvider<1, Tasking::SchedulePolicyFifo>
    {
    public:
        TestTask(const char* name) :
            Tasking::TaskProvider<1, Tasking::SchedulePolicyFifo>(scheduler, name), scheduler(policy)
        {
        }
        void
        execute(void) override
        {
        }

    private:
        Tasking::SchedulePolicyFifo policy;
        Tasking::SchedulerUnitTest scheduler;
    };
};

// Test works only if PLATFORM is none
TEST_F(TestUtils, MutexGuard)
{
    class TestMutex : public Tasking::Mutex
    {
    public:
#ifdef IS_NONE_PLATFORM
        using Mutex::occupied;
#endif
    };

    TestMutex mutex;
    {
        Tasking::MutexGuard guard(mutex);
#ifdef IS_NONE_PLATFORM
        EXPECT_TRUE(mutex.occupied);
#endif
    }
#ifdef IS_NONE_PLATFORM
    EXPECT_FALSE(mutex.occupied);
#else
    EXPECT_TRUE(true);
#endif
}

TEST_F(TestUtils, convertTaskId)
{
    const char* name = "Huge";
    TestTask task(name);
    EXPECT_TRUE(0 == memcmp(name, Tasking::IdConverter(task).name, 5));
}

TEST_F(TestUtils, convertChannelId)
{
    // Create a channel
    const char* name = "Well";
    Tasking::Channel channel(name);
    EXPECT_TRUE(0 == memcmp(name, Tasking::IdConverter(channel).name, 5));
}
