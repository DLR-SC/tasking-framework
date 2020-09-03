/*
 * testSchedulePolicyPriority.cpp
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
#include <schedulerUnitTest.h>
#include <schedulePolicyPriority.h>

class TestSchedulePolicyPriority : public ::testing::Test
{
public:
    TestSchedulePolicyPriority(void) :
                    scheduler(policy)
    {
    }
protected:
    class CheckTask : public Tasking::Task
    {
    public:
        CheckTask(Tasking::Scheduler& scheduler, Tasking::SchedulePolicyPriority::Settings settings) :
                        Task(scheduler, policyData, inputs), policyData(settings), impl(scheduler, policyData, *this, inputs)
        {
            // Nothing else to do.
        }
        /// Implement execute because it is necessary by default
        void execute(void)
        {
            // Nothing to do in this test
        }
        Tasking::InputArrayProvider<1u> inputs;
        Tasking::SchedulePolicyPriority::ManagementData policyData;
        Tasking::TaskImpl impl; // Tricky, because we need access to the private implementation, create a new one for the test.
    };

    class CheckTaskProvider : public Tasking::TaskProvider<1u, Tasking::SchedulePolicyPriority>
    {
    public:
        CheckTaskProvider(Tasking::Scheduler& scheduler, Tasking::SchedulePolicyPriority::Settings settings) :
                        Tasking::TaskProvider<1u, Tasking::SchedulePolicyPriority>(scheduler, settings, "Check"),
                        impl(scheduler, policyData, *this, inputs)
        {
            // Nothing else to do.
        }
        /// Implement execute because it is necessary by default
        void execute(void)
        {
            // Nothing to do in this test
        }
        Tasking::TaskImpl impl; // Tricky, because we need access to the private implementation, create a new one for the test.
    };

    Tasking::SchedulePolicyPriority policy;
    Tasking::SchedulerUnitTest scheduler;
};

TEST_F(TestSchedulePolicyPriority, Ordering)
{
    // After initialization there is no next task in the priority tree
    EXPECT_TRUE((policy.nextTask() == NULL));
    CheckTask task2(scheduler, Tasking::SchedulePolicyPriority::Settings(2u));
    EXPECT_TRUE(policy.queue(task2.impl));
    EXPECT_TRUE((policy.nextTask() == &task2.impl));
    EXPECT_TRUE((policy.nextTask() == NULL));
    EXPECT_TRUE(policy.queue(task2.impl)); // Tasks 2
    CheckTask task0a(scheduler, Tasking::SchedulePolicyPriority::Settings(0));
    EXPECT_FALSE(policy.queue(task0a.impl)); // Tasks 2 0a
    CheckTask task3a(scheduler, Tasking::SchedulePolicyPriority::Settings(3u));
    policy.queue(task3a.impl); // Tasks 3a 2 0a
    CheckTask task1(scheduler, Tasking::SchedulePolicyPriority::Settings(1u));
    policy.queue(task1.impl); // Tasks 3a 2 1 0a
    CheckTask task3b(scheduler, Tasking::SchedulePolicyPriority::Settings(3u));
    policy.queue(task3b.impl); // Tasks 3a 3b 2 1 0a
    CheckTask task0b(scheduler, Tasking::SchedulePolicyPriority::Settings(0u));
    policy.queue(task0b.impl); // Tasks 3a 3b 2 1 0a 0b
    EXPECT_TRUE((policy.nextTask() == &task3a.impl)); // Tasks 3b 2 1 0a 0b
    EXPECT_TRUE((policy.nextTask() == &task3b.impl)); // Tasks 2 1 0a 0b
    EXPECT_TRUE((policy.nextTask() == &task2.impl)); // Tasks 1 0a 0b
    EXPECT_TRUE((policy.nextTask() == &task1.impl)); // Tasks 0a 0b
    EXPECT_TRUE((policy.nextTask() == &task0a.impl)); // Tasks 0b
    EXPECT_TRUE((policy.nextTask() == &task0b.impl)); // Empty
    EXPECT_TRUE((policy.nextTask() == NULL));
}

TEST_F(TestSchedulePolicyPriority, UsingTaskProvider)
{
    // Create four task for the test
    CheckTaskProvider task1(scheduler, Tasking::SchedulePolicyPriority::Settings(1u));
    CheckTaskProvider task2a(scheduler, Tasking::SchedulePolicyPriority::Settings(2u));
    CheckTaskProvider task2b(scheduler, Tasking::SchedulePolicyPriority::Settings(2u));
    CheckTaskProvider task3(scheduler, Tasking::SchedulePolicyPriority::Settings(3u));
    // Queue the tasks ...
    policy.queue(task2a.impl);
    policy.queue(task3.impl);
    policy.queue(task1.impl);
    policy.queue(task2b.impl);
    // ...and check if they are order right in the queue, expect 3, 2a, 2b, 1
    EXPECT_TRUE((policy.nextTask() == &task3.impl));
    EXPECT_TRUE((policy.nextTask() == &task2a.impl));
    EXPECT_TRUE((policy.nextTask() == &task2b.impl));
    EXPECT_TRUE((policy.nextTask() == &task1.impl));
}
