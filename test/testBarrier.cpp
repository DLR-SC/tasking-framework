/*
 * testBarrier.cpp
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
#include <schedulerUnitTest.h>
#include <schedulePolicyLifo.h>
#include <task.h>
#include <taskBarrier.h>

class TestBarrier : public ::testing::Test
{
public:
    class CheckTask : public Tasking::TaskProvider<1u, Tasking::SchedulePolicyLifo>
    {
    public:
        CheckTask(Tasking::Scheduler& scheduler) :
                        TaskProvider(scheduler)
        {
            inputs[0].configure(1u);
        }
        virtual void execute(void)
        {
        }
        bool isActivated(void) const
        {
            return inputs[0].isActivated();
        }
    };

    Tasking::SchedulePolicyLifo schedulePolicy;
    Tasking::SchedulerUnitTest scheduler;
    CheckTask task;
    Tasking::Barrier barrier;
    TestBarrier(void) :
                    scheduler(schedulePolicy), task(scheduler)
    {
        task.configureInput(0u, barrier);
    }
};

TEST_F(TestBarrier, noActivationWithoutRegistration)
{
    barrier.push();
    EXPECT_FALSE(task.isActivated());
}

TEST_F(TestBarrier, normalOperation)
{
    barrier.increase();
    EXPECT_FALSE(task.isActivated());
    barrier.push();
    EXPECT_TRUE(task.isActivated());
}

TEST_F(TestBarrier, IncreaseMore)
{
    barrier.increase(2);
    EXPECT_FALSE(task.isActivated());
    barrier.push();
    EXPECT_FALSE(task.isActivated());
    barrier.push();
    EXPECT_TRUE(task.isActivated());
}

TEST_F(TestBarrier, IncreaseMeantime)
{
    barrier.increase(2);
    EXPECT_FALSE(task.isActivated());
    barrier.push();
    EXPECT_FALSE(task.isActivated());
    barrier.increase();
    EXPECT_FALSE(task.isActivated());
    barrier.push();
    EXPECT_FALSE(task.isActivated());
    barrier.push();
    EXPECT_TRUE(task.isActivated());
}

TEST_F(TestBarrier, ResetValue) {
    Tasking::Barrier barrierReset(2);
    task.configureInput(0u, barrierReset);
    EXPECT_FALSE(task.isActivated());
    barrierReset.push();
    EXPECT_FALSE(task.isActivated());
    barrierReset.push();
    EXPECT_TRUE(task.isActivated());
    task.reset();
    EXPECT_FALSE(task.isActivated());
    barrierReset.push();
    EXPECT_FALSE(task.isActivated());
    barrierReset.push();
    EXPECT_TRUE(task.isActivated());
}
