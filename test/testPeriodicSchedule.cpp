/*
 * testPeriodicSchedule.cpp
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
#include <taskPeriodicSchedule.h>
#include <schedulerUnitTest.h>
#include <schedulePolicyLifo.h>
#include <task.h>
#include <taskEvent.h>

class TestPeriodicSchedule : public ::testing::Test {
public:

    // Count task has to inputs configured with one arrival final.
    class CountTask : public Tasking::TaskProvider<1u, Tasking::SchedulePolicyLifo>
    {
    public:
        unsigned int counter;
        CountTask(Tasking::Scheduler& scheduler, const char *taskName = "") :
                        TaskProvider(scheduler, taskName), counter(0u)
        {
            inputs[0].configure(1u, true);
        }
        virtual void execute(void)
        {
            counter++;
        }
    };

    Tasking::SchedulePolicyLifo policy;
    Tasking::SchedulerUnitTest scheduler;
    Tasking::PeriodicSchedule periodicSchedule;
    Tasking::Event event;

    CountTask task;

    TestPeriodicSchedule(void): scheduler(policy), event(scheduler), task(scheduler, "PeriodicScheduleTask")
    {
        task.configureInput(0u, event);
    }
};

TEST_F(TestPeriodicSchedule, triggerTimeTrigger)
{
    event.trigger(1u); // Queue event, this shall be undone when periodic trigger is scheduled
    ASSERT_TRUE(event.isTriggered());

    // Configure schedule with period 10 ms and offset of 2 ms. Trigger fire at 6 ms in schedule and trigger the count task.
    Tasking::PeriodicScheduleTrigger trigger(6u);
    CountTask triggeredTask(scheduler);
    triggeredTask.configureInput(0, trigger);
    periodicSchedule.add(trigger);
    event.setPeriodicSchedule(10u, 2u, periodicSchedule);
    scheduler.start(true);

    // Before offset nothing should happens
    scheduler.schedule(1u);
    EXPECT_EQ(0u, triggeredTask.counter);
    // Cycle begins, also not trigger before 8 ms (schedule offset + trigger offset)
    scheduler.schedule(6u);
    EXPECT_EQ(0u, triggeredTask.counter);
    // Trigger time reached at 8 ms
    scheduler.schedule(1u);
    EXPECT_EQ(1u, triggeredTask.counter);
    // 10 ms period for schedule
    scheduler.schedule(9u);
    EXPECT_EQ(1u, triggeredTask.counter);
    scheduler.schedule(1u);
    EXPECT_EQ(2u, triggeredTask.counter);
    // The task connected to the task should never fired so long shall fire of event is not overridden.
    EXPECT_EQ(0u, task.counter);
}

TEST_F(TestPeriodicSchedule, timeTriggerOutsidePeriod)
{
    // Configure schedule with period 10 ms and trigger offset of 10 ms. This trigger is invalid.
    Tasking::PeriodicScheduleTrigger trigger(10u);
    CountTask triggeredTask(scheduler);
    triggeredTask.configureInput(0, trigger);
    periodicSchedule.add(trigger);
    event.setPeriodicSchedule(10u, 2u, periodicSchedule);
    scheduler.start(true);

    scheduler.schedule(13u); // If it is queued, it must fire yet.
    EXPECT_EQ(0u, triggeredTask.counter);
}

TEST_F(TestPeriodicSchedule, PeriodicScheduleWithoutTrigger)
{
    event.setPeriodicSchedule(10u, 2u, periodicSchedule);
    scheduler.start(true);
    // Without a trigger, the event should not be queued for first trigger
    EXPECT_FALSE(event.isTriggered());
}

TEST_F(TestPeriodicSchedule, manyTimeTriggers)
{
    // Configure schedule with period 10 ms and offset of 2 ms.
    // Five triggers are use at 2 ms, 2 x 6 ms, 7ms, and 8 ms
    Tasking::PeriodicScheduleTrigger trigger6a(6u);
    CountTask triggeredTask6a(scheduler);
    triggeredTask6a.configureInput(0, trigger6a);
    periodicSchedule.add(trigger6a);

    Tasking::PeriodicScheduleTrigger trigger8(8u);
    CountTask triggeredTask8(scheduler);
    triggeredTask8.configureInput(0, trigger8);
    periodicSchedule.add(trigger8);

    Tasking::PeriodicScheduleTrigger trigger2(2u);
    CountTask triggeredTask2(scheduler);
    triggeredTask2.configureInput(0, trigger2);
    periodicSchedule.add(trigger2);

    Tasking::PeriodicScheduleTrigger trigger7(7u);
    CountTask triggeredTask7(scheduler);
    triggeredTask7.configureInput(0, trigger7);
    periodicSchedule.add(trigger7);

    Tasking::PeriodicScheduleTrigger trigger6b(6u);
    CountTask triggeredTask6b(scheduler);
    triggeredTask6b.configureInput(0, trigger6b);
    periodicSchedule.add(trigger6b);

    event.setPeriodicSchedule(10u, 2u, periodicSchedule);
    scheduler.start(true);

    scheduler.schedule(3u); // 3ms, first trigger comes at 4 ms (2ms offset and 2ms of trigger)
    EXPECT_EQ(0u, triggeredTask2.counter);
    EXPECT_EQ(0u, triggeredTask6a.counter);
    EXPECT_EQ(0u, triggeredTask6b.counter);
    EXPECT_EQ(0u, triggeredTask7.counter);
    EXPECT_EQ(0u, triggeredTask8.counter);

    scheduler.schedule(1u); // 4ms, first trigger shall fire
    EXPECT_EQ(1u, triggeredTask2.counter);
    EXPECT_EQ(0u, triggeredTask6a.counter);
    EXPECT_EQ(0u, triggeredTask6b.counter);
    EXPECT_EQ(0u, triggeredTask7.counter);
    EXPECT_EQ(0u, triggeredTask8.counter);

    scheduler.schedule(3u); // 7 ms, next two triggers comes at 8 ms
    EXPECT_EQ(1u, triggeredTask2.counter);
    EXPECT_EQ(0u, triggeredTask6a.counter);
    EXPECT_EQ(0u, triggeredTask6b.counter);
    EXPECT_EQ(0u, triggeredTask7.counter);
    EXPECT_EQ(0u, triggeredTask8.counter);

    scheduler.schedule(1u); // 8 ms, the two 6ms triggers shall fire
    EXPECT_EQ(1u, triggeredTask2.counter);
    EXPECT_EQ(1u, triggeredTask6a.counter);
    EXPECT_EQ(1u, triggeredTask6b.counter);
    EXPECT_EQ(0u, triggeredTask7.counter);
    EXPECT_EQ(0u, triggeredTask8.counter);

    scheduler.schedule(1u); // 9 ms, the trigger at 7 ms shall fire
    EXPECT_EQ(1u, triggeredTask2.counter);
    EXPECT_EQ(1u, triggeredTask6a.counter);
    EXPECT_EQ(1u, triggeredTask6b.counter);
    EXPECT_EQ(1u, triggeredTask7.counter);
    EXPECT_EQ(0u, triggeredTask8.counter);

    scheduler.schedule(1u); // 10 ms, the last triggers fire at 10 ms
    EXPECT_EQ(1u, triggeredTask2.counter);
    EXPECT_EQ(1u, triggeredTask6a.counter);
    EXPECT_EQ(1u, triggeredTask6b.counter);
    EXPECT_EQ(1u, triggeredTask7.counter);
    EXPECT_EQ(1u, triggeredTask8.counter);

    scheduler.schedule(3u); // 13 ms, the first in the schedule shall now fire at 14 ms (2+10+2)
    EXPECT_EQ(1u, triggeredTask2.counter);
    EXPECT_EQ(1u, triggeredTask6a.counter);
    EXPECT_EQ(1u, triggeredTask6b.counter);
    EXPECT_EQ(1u, triggeredTask7.counter);
    EXPECT_EQ(1u, triggeredTask8.counter);

    scheduler.schedule(1u); // 14 ms
    EXPECT_EQ(2u, triggeredTask2.counter);
    EXPECT_EQ(1u, triggeredTask6a.counter);
    EXPECT_EQ(1u, triggeredTask6b.counter);
    EXPECT_EQ(1u, triggeredTask7.counter);
    EXPECT_EQ(1u, triggeredTask8.counter);}
