/*
 * testSignalChannel.cpp
 *
 * Copyright 2025 German Aerospace Center (DLR) SC
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

#include <channels/signalChannel.h>
#include <schedulerUnitTest.h>
#include <schedulePolicyFifo.h>
#include <task.h>

using Tasking::SchedulePolicyFifo;
using Tasking::SchedulerUnitTest;
using Tasking::SignalChannel;

using OneInputTask = Tasking::TaskProvider<1, SchedulePolicyFifo>;

struct FakeTask : public OneInputTask
{
    explicit FakeTask(Tasking::Scheduler& scheduler) : OneInputTask(scheduler, "FakeTask")
    {
        // Config input 0 to be triggered by only one activation and to trigger the task for every actication.
        inputs[0].configure(1, true);
    }

    void
    execute(void) override
    {
        if (flag)
        {
            value = 42;
            flag = false;
        }
        else
        {
            value = 100;
            flag = true;
        }
    }

    int value = 0;
    bool flag = true;
};

TEST(TestSignalChannel, activatesInput)
{
    SchedulePolicyFifo policy;
    SchedulerUnitTest scheduler(policy);

    SignalChannel channel(42u);
    EXPECT_EQ(channel.getChannelId(), 42u);

    FakeTask task(scheduler);
    task.configureInput(0, channel);
    EXPECT_EQ(task.value, 0);

    scheduler.start(true);
    scheduler.schedule();
    EXPECT_EQ(task.value, 0);

    channel.trigger();
    scheduler.schedule();
    ASSERT_EQ(task.value, 42);

    channel.trigger();
    scheduler.schedule();
    ASSERT_EQ(task.value, 100);

    channel.trigger();
    scheduler.schedule();
    ASSERT_EQ(task.value, 42);
}
