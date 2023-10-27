/*
 * accessor.h
 *
 * Copyright 2012-2023 German Aerospace Center (DLR) SC
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
#ifndef TASKING_ACCESSOR_H_
#define TASKING_ACCESSOR_H_

#include <scheduler.h>
#include <taskInput.h>
#include <taskEvent.h>
#include <taskChannel.h>
#include <task.h>

namespace Tasking
{

/// The good friend to allow access to private and protected members.
struct TaskingAccessor
{
    SchedulerImpl& getImpl(Scheduler& scheduler) const;
    void signal(Scheduler& scheduler) const;
    void synchronizeStart(Input& input) const;
    void synchronizeEnd(Input& input) const;
    void push(Tasking::Event& event) const;
    bool associateTo(Tasking::Channel& channel, InputImpl& input) const;
    void deassociate(Tasking::Channel& channel, InputImpl& input) const;
    void reset(Tasking::Channel& channel) const;
    void synchronizeStart(Tasking::Channel& channel, const Task* p_task, unsigned int volume) const;
    void synchronizeEnd(Tasking::Channel& channel, Task* p_task) const;
    void execute(Tasking::Task& task) const;
    void initialize(Tasking::Task& task) const;
};

inline SchedulerImpl&
TaskingAccessor::getImpl(Scheduler& scheduler) const
{
    return scheduler.impl;
}
inline void
TaskingAccessor::signal(Scheduler& scheduler) const
{
    scheduler.signal();
}
inline void
TaskingAccessor::synchronizeStart(Input& input) const
{
    input.synchronizeStart();
}
inline void
TaskingAccessor::synchronizeEnd(Input& input) const
{
    input.synchronizeEnd();
}
inline void
TaskingAccessor::push(Tasking::Event& event) const
{
    event.push();
}
inline bool
TaskingAccessor::associateTo(Tasking::Channel& channel, InputImpl& input) const
{
    return channel.associateTo(input);
}
inline void
TaskingAccessor::deassociate(Tasking::Channel& channel, InputImpl& input) const
{
    channel.deassociate(input);
}
inline void
TaskingAccessor::reset(Tasking::Channel& channel) const
{
    channel.reset();
}
inline void
TaskingAccessor::synchronizeStart(Tasking::Channel& channel, const Task* p_task, unsigned int volume) const
{
    channel.synchronizeStart(p_task, volume);
}
inline void
TaskingAccessor::synchronizeEnd(Tasking::Channel& channel, Task* p_task) const
{
    channel.synchronizeEnd(p_task);
}
inline void
TaskingAccessor::execute(Tasking::Task& task) const
{
    task.execute();
}
inline void
TaskingAccessor::initialize(Tasking::Task& task) const
{
    task.initialize();
}

} // namespace Tasking

#endif /* TASKING_ACCESSOR_H_ */
