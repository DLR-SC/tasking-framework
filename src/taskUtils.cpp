/*
 * taskUtils.cpp
 *
 * Copyright 2012-2020 German Aerospace Center (DLR) SC
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

#include <taskUtils.h>
#include <task.h>
#include <taskChannel.h>

Tasking::MutexGuard::MutexGuard(Tasking::Mutex& inMutex) : mutex(inMutex)
{
    mutex.enter();
}

// ------------------------------------

Tasking::MutexGuard::~MutexGuard()
{
    mutex.leave();
}

// ====================================

Tasking::IdConverter::IdConverter(const Tasking::Channel& channel)
{
    convertIdentificationToString<ChannelId>(channel.getChannelId(), name, sizeof(ChannelId) + 1u);
}

// ------------------------------------

Tasking::IdConverter::IdConverter(const Tasking::Task& task)
{
    convertIdentificationToString<TaskId>(task.getTaskId(), name, sizeof(TaskId) + 1u);
}
