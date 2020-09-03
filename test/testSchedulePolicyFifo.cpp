/*
 * testSchedulePolicyFifo.cpp
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
#include <schedulePolicyFifo.h>

class TestSchedulePolicyFifo : public ::testing::Test
{
public:
    TestSchedulePolicyFifo(void): scheduler(policy) {
    }
protected:
    class CheckTask : public Tasking::Task
    {
    public:
        CheckTask(Tasking::Scheduler& scheduler) :
                        Task(scheduler, policyData, inputs), impl(scheduler, policyData, *this, inputs)
        {
            // Nothing else to do.
        }
        /// Implement execute because it is necessary by default
        void execute(void)
        {
            // Nothing to do in this test
        }
        Tasking::InputArrayProvider<1u> inputs;
        Tasking::SchedulePolicyFifo::ManagementData policyData;
        Tasking::TaskImpl impl; // Tricky because we need access to the private implementation, so create a new one.
    };

    Tasking::SchedulePolicyFifo policy;
    Tasking::SchedulerUnitTest scheduler;
};

TEST_F(TestSchedulePolicyFifo, ordering) {
    // After initialization there is no next task in the FIFO
    EXPECT_TRUE((policy.nextTask() == NULL));
    CheckTask task1(scheduler);
    policy.queue(task1.impl);
    EXPECT_TRUE((policy.nextTask() == &task1.impl));
    EXPECT_TRUE((policy.nextTask() == NULL));
    // FIFO is empty now
    CheckTask task2(scheduler);
    policy.queue(task2.impl);
    CheckTask task3(scheduler);
    policy.queue(task3.impl);
    policy.queue(task1.impl);
    EXPECT_TRUE((policy.nextTask() == &task2.impl));
    EXPECT_TRUE((policy.nextTask() == &task3.impl));
    EXPECT_TRUE((policy.nextTask() == &task1.impl));
    EXPECT_TRUE((policy.nextTask() == NULL));
}
