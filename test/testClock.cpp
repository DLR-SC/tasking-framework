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

#include <impl/clock_impl.h>
#include <gtest/gtest.h>
#include <schedulePolicyLifo.h>
#include <scheduler.h>
#include <taskEvent.h>

class TestClock : public ::testing::Test
{
public:

    class TestScheduler : public Tasking::Scheduler
    {
    public:
        TestScheduler(Tasking::SchedulePolicy& schedulePolicy, Tasking::Clock& pClock) :
                        Scheduler(schedulePolicy, pClock), signalCount(0)
        {
        }
        virtual void signal(void)
        {
            ++signalCount;
        }
        virtual void waitUntilEmpty(void)
        {
        }
        virtual void setZeroTime(Tasking::Time)
        {
        }
        int signalCount;
    };

    /// Test event to access protected data of the event.
    class TestEvent : public Tasking::Event
    {
    public:
        // Mockup implementation, which is in normal case private part of event and not accessible
        Tasking::EventImpl impl;
        TestEvent(Tasking::Scheduler& scheduler, Tasking::Time time = 0u) :
                        Event(scheduler), impl(*this, scheduler)
        {
            impl.nextActivation_ms = time;
        }
        Tasking::EventImpl* getNext(void) const
        {
            return impl.next;
        }
        Tasking::EventImpl* getPrevious(void) const
        {
            return impl.previous;
        }
    };

    /// A clock implementation must provide the getTime method
    class ClockImplementation : public Tasking::Clock
    {
    public:
        ClockImplementation(Tasking::Scheduler& p_scheduler) :
                        Clock(p_scheduler), now(0u), waitingTime(0u)
        {
        }
        virtual Tasking::Time getTime(void) const
        {
            return now;
        }
        virtual void startTimer(Tasking::Time timeSpan)
        {
            waitingTime = timeSpan;
        }
        using Tasking::Clock::startAt;
        using Tasking::Clock::startIn;
        using Tasking::Clock::enqueue;
        using Tasking::Clock::enqueueHead;
        using Tasking::Clock::dequeue;
        using Tasking::Clock::dequeueAll;
        using Tasking::Clock::readFirstPending;
        using Tasking::Clock::getNextGapTime;
        using Tasking::Clock::getHeadTime;
        Tasking::Time now;
        Tasking::Time waitingTime;
    };

    Tasking::SchedulePolicyLifo policy;
    TestScheduler scheduler;
    ClockImplementation clock;
    TestEvent* events[9];

    TestClock(void) :
                    scheduler(policy, clock), clock(scheduler)
    {
        for (int i = 0; i < 9; ++i)
        {
            events[i] = NULL;
        }
    }

    ~TestClock(void)
    {
        for (int i = 0; i < 9; ++i)
        {
            if (events[i] != NULL)
            {
                delete events[i];
            }
        }
    }

    /// Three events at time point 1, 3, and 5
    void prepareFilledQueue(void)
    {
        int i = 0;
        for (Tasking::Time t = 1u; t < 6; t = t + 2)
        {
            for (int n = 0; n < 3; ++n)
            {
                events[i] = new TestEvent(scheduler, t);
                clock.enqueue(events[i]->impl);
                ++i;
            }
        }
    }
};

TEST_F(TestClock, NoPendingEventAfterInstantiation)
{
    EXPECT_TRUE(clock.isEmtpy());
    EXPECT_FALSE(clock.isPending());
    EXPECT_EQ(0u, clock.getHeadTime());
    EXPECT_TRUE(NULL == clock.readFirstPending());
}

TEST_F(TestClock, EnqueueHead)
{
    TestEvent event1(scheduler, 0u);
    clock.enqueueHead(event1.impl);
    EXPECT_FALSE(clock.isEmtpy());
    EXPECT_TRUE(clock.isPending());
    TestEvent event2(scheduler, 0u);
    clock.enqueueHead(event2.impl);
    EXPECT_TRUE(clock.isPending());
    TestEvent event3(scheduler, 0u);
    clock.enqueueHead(event3.impl);
    EXPECT_FALSE(clock.isEmtpy());
    EXPECT_TRUE(clock.isPending());
    EXPECT_TRUE(&event3.impl == clock.readFirstPending());
    EXPECT_FALSE(clock.isEmtpy());
    EXPECT_TRUE(clock.isPending());
    EXPECT_TRUE(&event2.impl == clock.readFirstPending());
    EXPECT_FALSE(clock.isEmtpy());
    EXPECT_TRUE(clock.isPending());
    EXPECT_TRUE(&event1.impl == clock.readFirstPending());
    EXPECT_TRUE(clock.isEmtpy());
    EXPECT_FALSE(clock.isPending());
    EXPECT_TRUE(NULL == clock.readFirstPending());
}

TEST_F(TestClock, EnqueueHeadRunningCondition)
{
    TestEvent event1(scheduler, 1u);
    clock.enqueueHead(event1.impl);
    clock.now = 1;
    TestEvent event2(scheduler, 0u);
    clock.enqueueHead(event2.impl);
    TestEvent event3(scheduler, 1u); // By a running condition order of timings is broken. But it is now
    clock.enqueueHead(event3.impl);
    EXPECT_TRUE(&event3.impl == clock.readFirstPending());
    EXPECT_TRUE(&event2.impl == clock.readFirstPending());
    EXPECT_TRUE(&event1.impl == clock.readFirstPending());
}

TEST_F(TestClock, EnqueueByTimeOneElement)
{
    TestEvent event1(scheduler, 1u);
    clock.enqueue(event1.impl);
    EXPECT_FALSE(clock.isEmtpy());
    EXPECT_FALSE(clock.isPending());
    EXPECT_EQ(1u, clock.getHeadTime());
    clock.now = 1u;
    EXPECT_TRUE(clock.isPending());
    EXPECT_TRUE(&event1.impl == clock.readFirstPending());
    EXPECT_FALSE(clock.isPending());
    EXPECT_EQ(0u, clock.getHeadTime());
    TestEvent event2(scheduler, 2u);
    clock.enqueue(event2.impl);
    EXPECT_FALSE(clock.isPending());
    clock.now = 2u;
    EXPECT_TRUE(clock.isPending());
    EXPECT_TRUE(&event2.impl == clock.readFirstPending());
    EXPECT_FALSE(clock.isPending());
}

TEST_F(TestClock, EnqueueSeveralElements)
{
    TestEvent event1a(scheduler, 1u);
    clock.enqueue(event1a.impl); // Expected queue 1a
    TestEvent event2a(scheduler, 2u);
    clock.enqueue(event2a.impl); // Expected queue 1a, 2a
    TestEvent event2b(scheduler, 2u);
    clock.enqueue(event2b.impl); // Expected queue 1a, 2a, 2b
    TestEvent event3a(scheduler, 3u);
    clock.enqueue(event3a.impl); // Expected queue 1a, 2a, 2b 3
    TestEvent event1b(scheduler, 1u);
    clock.enqueue(event1b.impl); // Expected queue 1a, 1b, 2a, 2b 3
    EXPECT_FALSE(clock.isPending());
    TestEvent event0a(scheduler, 0u);
    clock.enqueue(event0a.impl); // Expected queue 0a, 1a, 1b, 2a, 2b 3
    EXPECT_TRUE(clock.isPending());
    TestEvent event0b(scheduler, 0u);
    clock.enqueue(event0b.impl); // Expected queue 0a, 0b, 1a, 1b, 2a, 2b 3
    EXPECT_TRUE(clock.isPending());
    EXPECT_EQ(0u, clock.getHeadTime());
    EXPECT_TRUE(&event0a.impl == clock.readFirstPending());
    EXPECT_TRUE(clock.isPending());
    EXPECT_EQ(0u, clock.getHeadTime());
    EXPECT_TRUE(&event0b.impl == clock.readFirstPending());
    EXPECT_FALSE(clock.isPending());
    EXPECT_EQ(1u, clock.getHeadTime());
    clock.now++;
    EXPECT_TRUE(clock.isPending());
    EXPECT_TRUE(&event1a.impl == clock.readFirstPending());
    EXPECT_TRUE(clock.isPending());
    EXPECT_EQ(1u, clock.getHeadTime());
    EXPECT_TRUE(&event1b.impl == clock.readFirstPending());
    EXPECT_FALSE(clock.isPending());
    EXPECT_EQ(2u, clock.getHeadTime());
    clock.now++;
    EXPECT_TRUE(clock.isPending());
    EXPECT_TRUE(&event2a.impl == clock.readFirstPending());
    EXPECT_TRUE(clock.isPending());
    EXPECT_EQ(2u, clock.getHeadTime());
    EXPECT_TRUE(&event2b.impl == clock.readFirstPending());
    EXPECT_FALSE(clock.isPending());
    EXPECT_EQ(3u, clock.getHeadTime());
    clock.now++;
    EXPECT_TRUE(clock.isPending());
    EXPECT_TRUE(&event3a.impl == clock.readFirstPending());
    EXPECT_FALSE(clock.isPending());
    EXPECT_TRUE(NULL == clock.readFirstPending());
    EXPECT_EQ(0u, clock.getHeadTime());
    TestEvent event3b(scheduler, 3u);
    clock.enqueue(event3b.impl);
    EXPECT_TRUE(clock.isPending());
    EXPECT_TRUE(&event3b.impl == clock.readFirstPending());
    EXPECT_FALSE(clock.isPending());
}

TEST_F(TestClock, dequeueAll)
{
    TestEvent event0(scheduler, 0);
    TestEvent event1a(scheduler, 1);
    TestEvent event1b(scheduler, 1);
    TestEvent event2(scheduler, 2);
    clock.enqueue(event0.impl);
    clock.enqueue(event1a.impl);
    clock.enqueue(event1b.impl);
    clock.enqueue(event2.impl);
    EXPECT_TRUE(clock.isPending());
    clock.dequeueAll();
    EXPECT_FALSE(clock.isPending());
    clock.now = 3;
    EXPECT_FALSE(clock.isPending());
}

TEST_F(TestClock, dequeueHead)
{
    prepareFilledQueue();
    clock.dequeue(events[0]->impl);

    ASSERT_TRUE(events[0]->getNext() == NULL);
    ASSERT_TRUE(events[0]->getPrevious() == NULL);
    ASSERT_TRUE(events[1]->getNext() == &(events[2]->impl));
    ASSERT_TRUE(events[1]->getPrevious() == NULL);

    clock.now = 10;
    for (unsigned int i = 1; i < 9; ++i)
    {
        ASSERT_TRUE(&(events[i]->impl) == clock.readFirstPending());
    }
}

TEST_F(TestClock, dequeueMidStart)
{
    prepareFilledQueue();
    clock.dequeue(events[3]->impl);

    ASSERT_TRUE(events[2]->getNext() == &(events[4]->impl));
    ASSERT_TRUE(events[2]->getPrevious() == NULL);
    ASSERT_TRUE(events[3]->getNext() == NULL);
    ASSERT_TRUE(events[3]->getPrevious() == NULL);
    ASSERT_TRUE(events[4]->getNext() == &(events[5]->impl));
    ASSERT_TRUE(events[4]->getPrevious() == &(events[2]->impl));

    clock.now = 10;
    for (unsigned int i = 0; i < 3; ++i)
    {
        ASSERT_TRUE(&(events[i]->impl) == clock.readFirstPending());
    }
    for (unsigned int i = 4; i < 9; ++i)
    {
        ASSERT_TRUE(&(events[i]->impl) == clock.readFirstPending());
    }
    EXPECT_FALSE(clock.isPending());
}

TEST_F(TestClock, dequeueMidMid)
{
    prepareFilledQueue();
    clock.dequeue(events[4]->impl);

    ASSERT_TRUE(events[3]->getNext() == &(events[5]->impl));
    ASSERT_TRUE(events[3]->getPrevious() == &(events[2]->impl));
    ASSERT_TRUE(events[4]->getNext() == NULL);
    ASSERT_TRUE(events[4]->getPrevious() == NULL);
    ASSERT_TRUE(events[5]->getNext() == &(events[6]->impl));
    ASSERT_TRUE(events[5]->getPrevious() == &(events[2]->impl));

    clock.now = 10;
    for (unsigned int i = 0; i < 4; ++i)
    {
        ASSERT_TRUE(&(events[i]->impl) == clock.readFirstPending());
    }
    for (unsigned int i = 5; i < 9; ++i)
    {
        ASSERT_TRUE(&(events[i]->impl) == clock.readFirstPending());
    }
    EXPECT_FALSE(clock.isPending());
}

TEST_F(TestClock, dequeueMidLast)
{
    prepareFilledQueue();
    clock.dequeue(events[5]->impl);

    ASSERT_TRUE(events[4]->getNext() == &(events[6]->impl));
    ASSERT_TRUE(events[4]->getPrevious() == &(events[2]->impl));
    ASSERT_TRUE(events[5]->getNext() == NULL);
    ASSERT_TRUE(events[5]->getPrevious() == NULL);
    ASSERT_TRUE(events[6]->getNext() == &(events[7]->impl));
    ASSERT_TRUE(events[6]->getPrevious() == &(events[4]->impl));

    clock.now = 10;
    for (unsigned int i = 0; i < 5; ++i)
    {
        ASSERT_TRUE(&(events[i]->impl) == clock.readFirstPending());
    }
    for (unsigned int i = 6; i < 9; ++i)
    {
        ASSERT_TRUE(&(events[i]->impl) == clock.readFirstPending());
    }
    EXPECT_FALSE(clock.isPending());
}

TEST_F(TestClock, dequeueTail)
{
    prepareFilledQueue();
    clock.dequeue(events[8]->impl);

    ASSERT_TRUE(events[7]->getNext() == NULL);
    ASSERT_TRUE(events[7]->getPrevious() == &(events[5]->impl));
    ASSERT_TRUE(events[8]->getNext() == NULL);
    ASSERT_TRUE(events[8]->getPrevious() == NULL);

    TestEvent event(scheduler, 2u);
    clock.enqueue(event.impl);

    clock.now = 10;
    for (unsigned int i = 0; i < 3; ++i)
    {
        ASSERT_TRUE(&(events[i]->impl) == clock.readFirstPending());
    }
    ASSERT_TRUE(&event.impl == clock.readFirstPending());
    for (unsigned int i = 3; i < 8; ++i)
    {
        ASSERT_TRUE(&(events[i]->impl) == clock.readFirstPending());
    }
    EXPECT_FALSE(clock.isPending());
}

TEST_F(TestClock, startAtNow)
{
    TestEvent event(scheduler);
    clock.startAt(event.impl, 0u);
    EXPECT_EQ(1, scheduler.signalCount);
    EXPECT_TRUE(clock.isPending());
    EXPECT_TRUE(&event.impl == clock.readFirstPending());
    EXPECT_FALSE(clock.isPending());
}

TEST_F(TestClock, startInZeroDelay)
{
    TestEvent event(scheduler);
    clock.startIn(event.impl, 0u);
    EXPECT_EQ(1, scheduler.signalCount);
    EXPECT_TRUE(clock.isPending());
    EXPECT_TRUE(&event.impl == clock.readFirstPending());
    EXPECT_FALSE(clock.isPending());
}

TEST_F(TestClock, startAtDelayed)
{
    TestEvent event(scheduler);
    clock.startAt(event.impl, 1u);
    EXPECT_EQ(0, scheduler.signalCount);
    EXPECT_EQ(1u, clock.waitingTime);
    EXPECT_FALSE(clock.isPending());
    EXPECT_TRUE(NULL == clock.readFirstPending());
    clock.now = 1u;
    EXPECT_TRUE(clock.isPending());
    EXPECT_TRUE(&event.impl == clock.readFirstPending());
    EXPECT_FALSE(clock.isPending());
}

TEST_F(TestClock, startInDelayed)
{
    TestEvent event(scheduler);
    clock.startIn(event.impl, 1u);
    EXPECT_EQ(0, scheduler.signalCount);
    EXPECT_EQ(1u, clock.waitingTime);
    EXPECT_FALSE(clock.isPending());
    EXPECT_TRUE(NULL == clock.readFirstPending());
    clock.now = 1u;
    EXPECT_TRUE(clock.isPending());
    EXPECT_TRUE(&event.impl == clock.readFirstPending());
    EXPECT_FALSE(clock.isPending());
}

TEST_F(TestClock, startInDelayedAndAtZero)
{
    TestEvent event1(scheduler);
    clock.startIn(event1.impl, 2u);
    EXPECT_EQ(0, scheduler.signalCount);
    EXPECT_EQ(2u, clock.waitingTime);
    clock.now = 1u;
    EXPECT_FALSE(clock.isPending());
    TestEvent event2(scheduler);
    clock.startAt(event2.impl, 1u); // Should trigger right now
    EXPECT_EQ(1, scheduler.signalCount);
    EXPECT_EQ(2u, clock.waitingTime);
    EXPECT_TRUE(clock.isPending());
    EXPECT_TRUE(&event2.impl == clock.readFirstPending());
    EXPECT_FALSE(clock.isPending());
    EXPECT_TRUE(NULL == clock.readFirstPending());
    clock.now = 2u;
    EXPECT_TRUE(clock.isPending());
    EXPECT_TRUE(&event1.impl == clock.readFirstPending());
    EXPECT_FALSE(clock.isPending());
}

TEST_F(TestClock, startTimerCall)
{
    // Event two is fired in 2 ms, so timer is adjusted to wake up in 2 ms
    Tasking::Event event2(scheduler);
    event2.trigger(2u);
    EXPECT_EQ(2u, clock.waitingTime);
    // Event immediate is fired directly, scheduler is signaled without the clock by now
    Tasking::Event eventImmediate(scheduler);
    eventImmediate.trigger();
    EXPECT_EQ(2u, clock.waitingTime);
    // Event one is fired in 1 ms. Its earlier than event two, so wake up time has to adjusted
    Tasking::Event event1(scheduler);
    event1.trigger(1u);
    EXPECT_EQ(1u, clock.waitingTime);
    // Event one is fired in 3 ms. Its later, so no adjustment is needed.
    Tasking::Event event3(scheduler);
    event3.trigger(3u);
    EXPECT_EQ(1u, clock.waitingTime);
}

TEST_F(TestClock, notQueuedTwice) {
    TestEvent event(scheduler);
    clock.startIn(event.impl, 1u);
    clock.startIn(event.impl, 1u);
    clock.now = 1u;
    // Intermediate step forward on next millisecond window
    clock.startIn(event.impl, 1u);
    EXPECT_TRUE(&event.impl == clock.readFirstPending());
    clock.now = 2u;
    EXPECT_FALSE(&event.impl == clock.readFirstPending());
}

TEST_F(TestClock, notQueuedTwiceFixTime) {
    TestEvent event(scheduler);
    clock.startIn(event.impl, 1u);
    clock.startIn(event.impl, 1u);
    clock.now = 1u;
    // Intermediate step forward on next millisecond window
    clock.startIn(event.impl, 0u);
    EXPECT_TRUE(&event.impl == clock.readFirstPending());
    clock.now = 2u;
    EXPECT_FALSE(&event.impl == clock.readFirstPending());
}

TEST_F(TestClock, triggerOnlyOnce)
{
    Tasking::Event event(scheduler);
    event.trigger(2u);
    event.trigger(1u);
    clock.now = 1;
    ASSERT_TRUE(clock.isPending());
    EXPECT_TRUE(&event == &(clock.readFirstPending()->parent));
    event.trigger(1u); // At time point 2
    event.trigger(2u); // At time point 3
    clock.now = 2;
    EXPECT_FALSE(clock.isPending());
    clock.now = 3;
    EXPECT_TRUE(clock.isPending());
    EXPECT_TRUE(&event == &(clock.readFirstPending()->parent));
}

TEST_F(TestClock, getDelay)
{
    // On empty clock queue the is no next gap time
    EXPECT_EQ(0u, clock.getNextGapTime());
    // With only one element there is no gap time
    Tasking::Event event5a(scheduler);
    event5a.trigger(5u); // Time point 5
    EXPECT_EQ(0u, clock.getNextGapTime());
    // Also when only elements with the same time are in the clock queue
    Tasking::Event event5b(scheduler);
    event5b.trigger(5u); // Time points 5, 5
    EXPECT_EQ(0u, clock.getNextGapTime());
    // Now put in an event after the current heads
    Tasking::Event event15(scheduler);
    event15.trigger(15u); // Time points 5, 5, 15
    EXPECT_EQ(10u, clock.getNextGapTime());
    // Now put in one time before the equal events
    Tasking::Event event2(scheduler);
    event2.trigger(2u); // Time points 2, 5, 5, 15
    EXPECT_EQ(3u, clock.getNextGapTime());
    // Behind first block an event
    Tasking::Event event9(scheduler);
    event9.trigger(9u); // Time points 2, 5, 5, 9, 15
    EXPECT_EQ(3u, clock.getNextGapTime());
    // Now consume the elements step by step
    clock.now = 15; // All pending, but it make no difference for next gap time
    clock.readFirstPending(); // Time points 5, 5, 9, 15
    EXPECT_EQ(4u, clock.getNextGapTime());
    clock.readFirstPending(); // Time points 5, 9, 15
    EXPECT_EQ(4u, clock.getNextGapTime());
    clock.readFirstPending(); // Time points 9, 15
    EXPECT_EQ(6u, clock.getNextGapTime());
    clock.readFirstPending(); // Time point 15
    EXPECT_EQ(0u, clock.getNextGapTime());
    clock.readFirstPending(); // Empty again
    EXPECT_EQ(0u, clock.getNextGapTime());
}
