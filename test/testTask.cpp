/*
 * testTask.cpp
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
#include <cstring>

#include <task.h>
#include <taskInput.h>
#include <taskChannel.h>
#include <schedulerUnitTest.h>
#include <schedulePolicyLifo.h>

class TestTask : public ::testing::Test
{
public:

    TestTask(void) :
                    scheduler(policy), checker(scheduler)
    {
        // input 1 is not used in every test case, don't add it here. Add it in test case
        checker.configureInput(0, msg[0]);
    }

    ~TestTask(void)
    {
    }

    class CheckChannel : public Tasking::Channel
    {
    public:
        CheckChannel(void) :
                        Channel(1), resets(0)
        {
            // Nothing else to do.
        }

        using Tasking::Channel::push;

        virtual void reset(void)
        {
            ++resets;
        }
        int resets;
    };

    /// Receiving task which count the executions. The task hold two inputs
    class CheckTask : public Tasking::TaskProvider<2u, Tasking::SchedulePolicyLifo>
    {
    public:
        CheckTask(Tasking::Scheduler& scheduler, const Tasking::TaskId id = 0u) :
                        TaskProvider(scheduler, id), calls(0)
        {
            // Configurate input settings
            inputs[0].configure(1u);
            inputs[1].configure(0u);
        }

        CheckTask(Tasking::Scheduler& scheduler, const char *taskName) :
                        TaskProvider(scheduler, taskName), calls(0)
        {
        }

        /// Overload execute to count executions
        void execute(void)
        {
            calls++;
        }
        /// Provide call to protected method getInput.
        using Tasking::TaskProvider<2u, Tasking::SchedulePolicyLifo>::inputs;
        const Tasking::Input& getInput(unsigned int id)
        {
            return inputs[id];
        }
        CheckChannel* getChannel(unsigned int key) const
        {
            return Task::getChannel<CheckChannel>(key);
        }
        /// Number of executions
        int calls;
    };

    class NameCheckTask: public Tasking::Task
    {
    public:
        NameCheckTask(Tasking::Scheduler& scheduler, const char *taskName) :
            Task(scheduler, policy, inputs, taskName)
    {
    }
    protected:
        void execute(void) override
        {
        }
    private:
        /// Scheduling policy
        Tasking::SchedulePolicyLifo::ManagementData policy;
        /// Inputs
        Tasking::InputArrayProvider<1> inputs;
    };

    Tasking::SchedulePolicyLifo policy;
    Tasking::SchedulerUnitTest scheduler;
    /// Task definition for unit tests
    CheckTask checker;
    /// Two messages to connect with the task.
    CheckChannel msg[2];
};

TEST_F(TestTask, configure)
{
    CheckTask task(scheduler);
    EXPECT_FALSE(task.isValid());
    EXPECT_EQ(NULL, task.getInput(0u).getChannel<CheckChannel>());
    EXPECT_EQ(NULL, task.getChannel(0u));
    task.configureInput(0u, msg[0]);
    EXPECT_FALSE(task.isValid());
    EXPECT_EQ(msg, task.getInput(0u).getChannel<CheckChannel>());
    EXPECT_EQ(NULL, task.getInput(1u).getChannel<CheckChannel>());
    EXPECT_EQ(msg, task.getChannel(0u));
    EXPECT_EQ(NULL, task.getChannel(1u));
    task.configureInput(1u, msg[1]);
    EXPECT_TRUE(task.isValid());
    EXPECT_EQ(msg, task.getInput(0u).getChannel<CheckChannel>());
    EXPECT_EQ(msg + 1, task.getInput(1u).getChannel<CheckChannel>());
    EXPECT_EQ(msg, task.getChannel(0u));
    EXPECT_EQ(msg + 1, task.getChannel(1u));

}

TEST_F(TestTask, reset)
{
    checker.configureInput(1, msg[1]);
    // Push on both channels and then reset. Inputs should be not activated and to all channels reset is called
    msg[0].push();
    msg[1].push();
    checker.reset();
    EXPECT_FALSE(checker.getInput(0).isActivated());
    EXPECT_EQ(1, msg[0].resets);
    EXPECT_TRUE(checker.getInput(1).isActivated());
    EXPECT_EQ(0u, checker.getInput(1).getActivations());
    EXPECT_EQ(1, msg[1].resets);
}

TEST_F(TestTask, resetWithSynchronizedInput)
{
    msg[0].push();
    msg[0].push();
    checker.reset();
    EXPECT_TRUE(checker.getInput(0).isActivated());
    EXPECT_EQ(1, msg[0].resets);
    checker.reset();
    EXPECT_FALSE(checker.getInput(0).isActivated());
    EXPECT_EQ(2, msg[0].resets);
}

TEST_F(TestTask, isExecutedSingle)
{
    scheduler.start();
    EXPECT_EQ(0, checker.calls);
    msg[0].push();
    scheduler.schedule();
    EXPECT_EQ(1, checker.calls);
}

TEST_F(TestTask, isExecutedMulti)
{
    scheduler.start();
    checker.configureInput(1, msg[1]);
    checker.reset();
    EXPECT_EQ(0, checker.calls);
    msg[1].push(); // Optional input will not trigger because input 0 is not activated
    scheduler.schedule();
    EXPECT_EQ(0, checker.calls);
    msg[0].push(); // Activation of input 0 trigger task
    scheduler.schedule();
    EXPECT_EQ(1, checker.calls);
    msg[0].push(); // Activation of input 0 trigger task because second input is optional
    scheduler.schedule();
    EXPECT_EQ(2, checker.calls);
}

TEST_F(TestTask, testTaskId)
{
    CheckTask task(scheduler, 84u);
    EXPECT_EQ(84u, task.getTaskId());
    task.setTaskId(42u);
    EXPECT_EQ(42u, task.getTaskId());
}

TEST_F(TestTask, testTaskProviderName)
{
    CheckTask task(scheduler, "5,-$");
    EXPECT_EQ(0x352C2D24u, task.getTaskId());
    task.setTaskName("foo");
    EXPECT_EQ(0x666F6F00u, task.getTaskId());
    CheckTask longNameTask(scheduler, "LongName");
    EXPECT_EQ(0x4C6F6E67u, longNameTask.getTaskId());
}

TEST_F(TestTask, testTaskName)
{
    NameCheckTask task(scheduler, "_6.U");
    char name[5];
    Tasking::convertTaskIdToString(task.getTaskId(), name, 5);
    EXPECT_TRUE((strncmp(name, "_6.U", 5)==0));
    Tasking::convertTaskIdToString(task.getTaskId(), name, 4);
    EXPECT_TRUE((strncmp(name, "_6.", 5)==0));
}
