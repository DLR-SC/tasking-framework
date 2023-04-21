/*
 * testTaskGroup.cpp
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

#include <pthread.h>
#include <gtest/gtest.h>
#include <taskGroup.h>
#include <taskChannel.h>
#include <task.h>
#include <schedulerUnitTest.h>
#include <schedulePolicyLifo.h>

class TestTaskGroup : public ::testing::Test
{
public:
    TestTaskGroup(void) : scheduler(policy), checker1(scheduler), checker2(scheduler)
    {
        checker1.configureInput(0, msg[0]);
        scheduler.start();
    }

    class CheckMessage : public Tasking::Channel
    {
    public:
        void
        push(void)
        {
            Channel::push();
        }
    };

    /// Receiving task which count the executions
    class CheckTask : public Tasking::TaskProvider<1, Tasking::SchedulePolicyLifo>
    {
    public:
        CheckTask(Tasking::Scheduler& scheduler) :
            TaskProvider<1, Tasking::SchedulePolicyLifo>(scheduler), calls(0), resets(0), doTaskReset(true)
        {
            inputs[0].configure(1);
        }
        /// Overload execute to count executions
        void
        execute(void)
        {
            calls++;
        }
        /// Overload reset to reset count of executions to zero
        void
        reset(void)
        {
            resets++;
            if (doTaskReset)
            {
                Task::reset();
            }
        }
        /// Number of executions
        int calls;
        /// Number of reset calls
        int resets;
        /// Allow reset of task when reset is called
        bool doTaskReset;
    };

    /// Test group for two tasks.
    class TestGroup : public Tasking::GroupProvider<2>
    {
    public:
        using Tasking::Group::areAllExecuted;
    };

    Tasking::SchedulePolicyLifo policy;
    Tasking::SchedulerUnitTest scheduler;
    CheckTask checker1;
    CheckTask checker2;
    CheckMessage msg[2];
    TestGroup group;

    void
    preparefullConnection(void)
    {
        checker2.configureInput(0, msg[1]);
        group.join(checker1);
        group.join(checker2);
    }
};

TEST_F(TestTaskGroup, isValid)
{
    EXPECT_FALSE(group.isValid());
    group.join(checker1);
    EXPECT_FALSE(group.isValid());
    group.join(checker2); // Checker 2 is not configured correctly, so still not valid
    EXPECT_FALSE(group.isValid());
    checker2.configureInput(0, msg[1]);
    EXPECT_TRUE(group.isValid());
}

TEST_F(TestTaskGroup, areAllExecuted)
{
    preparefullConnection();
    checker1.doTaskReset = false;
    checker2.doTaskReset = false;
    EXPECT_FALSE(group.areAllExecuted());
    msg[0].push();
    scheduler.schedule();
    EXPECT_FALSE(group.areAllExecuted());
    msg[1].push();
    scheduler.schedule();
    EXPECT_TRUE(group.areAllExecuted());
}

TEST_F(TestTaskGroup, executeCylce)
{
    preparefullConnection();
    msg[1].push();
    scheduler.schedule();
    EXPECT_EQ(1, checker2.resets); // One from start up
    msg[0].push();
    scheduler.schedule();
    EXPECT_EQ(2, checker1.resets);
    EXPECT_EQ(2, checker2.resets);
}
