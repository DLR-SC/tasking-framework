/*
 * testTaskEvent.cpp
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
#include <taskEvent.h>
#include <task.h>
#include <schedulerUnitTest.h>
#include <schedulePolicyLifo.h>

class TestTaskEvent : public ::testing::Test
{
public:
    // Count task has to inputs configured with one notification and final.
    class CountTask : public Tasking::TaskProvider<2u, Tasking::SchedulePolicyLifo>
    {
    public:
        unsigned int counter;
        CountTask(Tasking::Scheduler& scheduler) : TaskProvider(scheduler), counter(0u)
        {
            inputs[0].configure(1u, true);
            inputs[0].setSynchron(false);
            inputs[1].configure(1u, true);
            inputs[1].setSynchron(false);
        }
        virtual void
        execute(void)
        {
            counter++;
        }
    };

    // Overload onFire to check execution by scheduler
    class ControlledEvent : public Tasking::Event
    {
    public:
        ControlledEvent(Tasking::Scheduler& scheduler) :
            Event(scheduler), onFireCounter(0u), shallFireCounter(0u), allowFire(true)
        {
        }
        ControlledEvent(Tasking::Scheduler& scheduler, const char* name) :
            Event(scheduler, name), onFireCounter(0u), shallFireCounter(0u), allowFire(true)
        {
        }
        void
        onFire(void) override
        {
            ++onFireCounter;
        }
        bool
        shallFire(void) override
        {
            ++shallFireCounter;
            return allowFire;
        }
        unsigned int onFireCounter;
        unsigned int shallFireCounter;
        bool allowFire;
    };

    Tasking::SchedulePolicyLifo policy;
    Tasking::SchedulerUnitTest scheduler;
    ControlledEvent event;
    ControlledEvent unallowedEvent;
    CountTask task;

    TestTaskEvent(void) : scheduler(policy), event(scheduler), unallowedEvent(scheduler), task(scheduler)
    {
        task.configureInput(0u, event);
        task.configureInput(1u, unallowedEvent);
        unallowedEvent.allowFire = false;
        scheduler.setZeroTime(0u); // Not necessary, but the empty function reduce test coverage.
        scheduler.start();
    }
};

TEST_F(TestTaskEvent, NoCallAfterConstructing)
{
    scheduler.schedule();
    EXPECT_EQ(0u, task.counter);
}

TEST_F(TestTaskEvent, EventName)
{
    ControlledEvent namedEvent(scheduler, "Jerk");
    EXPECT_EQ(0x4A65726Bu, namedEvent.getChannelId());
}

TEST_F(TestTaskEvent, triggerImmediately)
{
    // Trigger right now. Task should executed
    event.trigger(); // Trigger right now. Task should executed
    scheduler.schedule();
    EXPECT_EQ(1u, task.counter);
    unallowedEvent.trigger(); // Trigger right now. Task should not executed because shallFire is false
    scheduler.schedule();
    EXPECT_EQ(1u, task.counter);
    EXPECT_EQ(1u, unallowedEvent.shallFireCounter);
}

TEST_F(TestTaskEvent, timedTrigger)
{
    // Trigger in 10 ms. No trigger before or after.
    event.trigger(10u);
    unallowedEvent.trigger(10u);
    scheduler.schedule(9u);
    EXPECT_EQ(0u, task.counter);
    scheduler.schedule(1u); // 10 ms over
    EXPECT_EQ(1u, task.counter);
    EXPECT_EQ(1u, unallowedEvent.shallFireCounter);
    scheduler.schedule(1000u * 60u * 60u * 24u * 365u); // Time step by one year
    EXPECT_EQ(1u, task.counter);
    EXPECT_EQ(1u, unallowedEvent.shallFireCounter);
}

TEST_F(TestTaskEvent, isTriggered)
{
    EXPECT_FALSE(event.isTriggered());
    event.trigger(); // By this the event is not queued.
    EXPECT_FALSE(event.isTriggered());
    event.trigger(1u);
    EXPECT_TRUE(event.isTriggered());
    event.stop();
    EXPECT_FALSE(event.isTriggered());
    event.trigger(1u);
    EXPECT_TRUE(event.isTriggered());
    scheduler.schedule(1u);
    EXPECT_FALSE(event.isTriggered());
    event.trigger(1u);
    event.trigger(); // Immediate trigger should also dequeue
    EXPECT_FALSE(event.isTriggered());
}

TEST_F(TestTaskEvent, StoppTrigger)
{
    event.setRelativeTiming(50u);
    event.reset();
    scheduler.schedule(10u);
    event.stop();
    scheduler.schedule(50u);
    EXPECT_EQ(0u, task.counter);
}

TEST_F(TestTaskEvent, PeriodicTiming)
{
    event.setPeriodicTiming(5u, 3u);
    unallowedEvent.setPeriodicTiming(2u, 4u);
    scheduler.schedule(2u); // 2 ms
    EXPECT_EQ(0u, task.counter);
    EXPECT_EQ(0u, unallowedEvent.shallFireCounter);
    scheduler.schedule(1u); // 3 ms == offset
    EXPECT_EQ(1u, task.counter);
    EXPECT_EQ(0u, unallowedEvent.shallFireCounter);
    scheduler.schedule(1u); // 4 ms
    EXPECT_EQ(1u, task.counter);
    EXPECT_EQ(1u, unallowedEvent.shallFireCounter);
    scheduler.schedule(1u); // 5 ms
    EXPECT_EQ(1u, task.counter);
    EXPECT_EQ(1u, unallowedEvent.shallFireCounter);
    scheduler.schedule(2u); // 7 ms
    EXPECT_EQ(1u, task.counter);
    EXPECT_EQ(2u, unallowedEvent.shallFireCounter);
    scheduler.schedule(1u); // 8 ms == offset + period
    EXPECT_EQ(2u, task.counter);
    EXPECT_EQ(3u, unallowedEvent.shallFireCounter);
    scheduler.schedule(4u); // 12 ms
    EXPECT_EQ(2u, task.counter);
    EXPECT_EQ(5u, unallowedEvent.shallFireCounter);
    scheduler.schedule(1u); // 13 ms == offset + 2*period
    EXPECT_EQ(3u, task.counter);
    EXPECT_EQ(5u, unallowedEvent.shallFireCounter);
    event.stop();
    unallowedEvent.stop();
    scheduler.schedule(5u); // 18 ms == offset + 3*period but stop
    EXPECT_EQ(3u, task.counter);
    EXPECT_EQ(5u, unallowedEvent.shallFireCounter);
}

TEST_F(TestTaskEvent, PeriodicTimingReset)
{
    // Additional reset operation has no effect on periodic timing, e.g. by final inputs
    event.setPeriodicTiming(5u, 2u);
    scheduler.schedule(4u); // 4 ms
    EXPECT_EQ(1u, task.counter);
    task.reset();
    scheduler.schedule(2u); // 6 ms to early
    EXPECT_EQ(1u, task.counter);
    scheduler.schedule(1u); // 7 ms == offset + period
    EXPECT_EQ(2u, task.counter);
}

TEST_F(TestTaskEvent, PeriodicTimingNotEndedByTrigger)
{
    event.setPeriodicTiming(5u, 3u);
    scheduler.schedule(4u); // 4 ms
    EXPECT_EQ(1u, task.counter);
    event.trigger(1u); // Should not fired because we are configured as periodic timer.
    scheduler.schedule(1u); // 5 ms
    EXPECT_EQ(1u, task.counter);
    event.trigger(); // Should not fired because we are configured as periodic timer.
    scheduler.schedule(); // 5 ms
    EXPECT_EQ(1u, task.counter);
    scheduler.schedule(3u); // 8 ms
    EXPECT_EQ(2u, task.counter);
}

TEST_F(TestTaskEvent, ZeroPeriod)
{
    event.setPeriodicTiming(0u, 5u);
    scheduler.schedule(4u); // 4 ms
    EXPECT_EQ(0u, task.counter);
    scheduler.schedule(1u); // 5 ms
    EXPECT_EQ(1u, task.counter);
    scheduler.schedule(10u); // 15 ms
    EXPECT_EQ(1u, task.counter);
    event.setPeriodicTiming(0u, 14u); // Time point in the history, immediate start
    scheduler.schedule(); // 15 ms
    EXPECT_EQ(2u, task.counter);
}

TEST_F(TestTaskEvent, RelativeTiming)
{
    event.setRelativeTiming(5u);
    unallowedEvent.setRelativeTiming(6u);
    scheduler.schedule(5u); // 5 ms No run, because no reset yet
    EXPECT_EQ(0u, task.counter);
    EXPECT_EQ(0u, unallowedEvent.shallFireCounter);
    scheduler.schedule(7u); // 12 ms
    EXPECT_EQ(0u, task.counter);
    EXPECT_EQ(0u, unallowedEvent.shallFireCounter);
    task.reset();
    scheduler.schedule(4u); // 16 ms, no run because reset was at 12 ms, so start at 17 ms and unallowed event at 18 ms
    EXPECT_EQ(0u, task.counter);
    EXPECT_EQ(0u, unallowedEvent.shallFireCounter);
    scheduler.schedule(1u); // 17 ms
    EXPECT_EQ(1u, task.counter);
    EXPECT_EQ(0u, unallowedEvent.shallFireCounter);
    scheduler.schedule(4u); // 21 ms, next start is at 22 ms (17+5) and unallowed event at 23 ms (17+6)
    EXPECT_EQ(1u, task.counter);
    EXPECT_EQ(0u, unallowedEvent.shallFireCounter);
    scheduler.schedule(2u); // 23 ms, trigger was at 22 ms. It#s later, so also unallowed event was checked by delay
    EXPECT_EQ(2u, task.counter);
    EXPECT_EQ(1u, unallowedEvent.shallFireCounter);
    scheduler.schedule(4u); // 27 ms, last reset was delayed at 23 ms, expect next wake up at 28 ms and 29 ms
    EXPECT_EQ(2u, task.counter);
    EXPECT_EQ(1u, unallowedEvent.shallFireCounter);
    scheduler.schedule(1u); // 28 ms, it must trigger
    EXPECT_EQ(3u, task.counter);
    EXPECT_EQ(1u, unallowedEvent.shallFireCounter);
    event.stop();
    scheduler.schedule(1u); // 33 ms, event is stopped
    EXPECT_EQ(3u, task.counter);
    EXPECT_EQ(1u, unallowedEvent.shallFireCounter);
}

TEST_F(TestTaskEvent, RelativeTimingNotEndedByTrigger)
{
    event.setRelativeTiming(5u);
    task.reset();
    scheduler.schedule(2u); // 2 ms
    EXPECT_EQ(0u, task.counter);
    event.trigger(1u); // Should not fired because we are configured as relative timer.
    scheduler.schedule(1u); // 3 ms
    EXPECT_EQ(0u, task.counter);
    event.trigger(); // Should not fired because we are configured as relative timer.
    scheduler.schedule(); // 3 ms
    EXPECT_EQ(0u, task.counter);
    scheduler.schedule(2u); // 5 ms
    EXPECT_EQ(1u, task.counter);
}

TEST_F(TestTaskEvent, ExecutionsWithDelay)
{
    event.setPeriodicTiming(5u, 5u);
    task.reset();
    scheduler.schedule(9u);
    EXPECT_EQ(1u, task.counter); // At 5 ms
    scheduler.schedule(11u);
    // It is performed only one further time. Activations at 15 ms and 20 ms are lost by the blocking.
    EXPECT_EQ(2u, task.counter);
    scheduler.schedule(5u);
    EXPECT_EQ(3u, task.counter);
}

TEST_F(TestTaskEvent, shallFireAndOnFire)
{
    event.allowFire = false;
    event.trigger(1u);
    scheduler.schedule(1u);
    EXPECT_EQ(0u, event.onFireCounter);
    event.setPeriodicTiming(1u, 1u);
    scheduler.schedule(1u);
    EXPECT_EQ(0u, event.onFireCounter);
    event.stop();
    event.setRelativeTiming(1u);
    task.reset();
    scheduler.schedule(1u);
    EXPECT_EQ(0u, event.onFireCounter);
    event.stop();

    event.allowFire = true;
    event.trigger(1u);
    scheduler.schedule(1u);
    EXPECT_EQ(1u, event.onFireCounter);
    event.setPeriodicTiming(1u, 1u);
    scheduler.schedule(1u);
    EXPECT_EQ(2u, event.onFireCounter);
    event.stop();
    event.setRelativeTiming(1u);
    task.reset();
    scheduler.schedule(1u);
    EXPECT_EQ(3u, event.onFireCounter);
}

TEST_F(TestTaskEvent, requestTime)
{
    EXPECT_EQ(0u, event.now());
    scheduler.schedule(2u);
    EXPECT_EQ(2u, event.now());
}

TEST_F(TestTaskEvent, configurePeriodicOnQueuedEvent)
{
    event.trigger(1u); // Queue event
    event.setPeriodicTiming(2u, 3u);
    scheduler.schedule(1u);
    EXPECT_EQ(0u, task.counter);
    scheduler.schedule(1u);
    EXPECT_EQ(0u, task.counter);
    scheduler.schedule(1u);
    EXPECT_EQ(1u, task.counter);
}

TEST_F(TestTaskEvent, configureRelativeOnQueuedEvent)
{
    event.trigger(1u); // Queue event
    event.setRelativeTiming(3u); // Dequeue and start relative timing on reset
    event.reset(); // Queue Event with start at 3 ms
    scheduler.schedule(1u);
    EXPECT_EQ(0u, task.counter);
    scheduler.schedule(1u);
    EXPECT_EQ(0u, task.counter);
    scheduler.schedule(1u);
    EXPECT_EQ(1u, task.counter);
}
