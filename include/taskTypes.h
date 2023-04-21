/*
 * taskTypes.h
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

#ifndef TASKTYPES_H_
#define TASKTYPES_H_

#include <cstdlib>
#include <stdint.h>
#include <limits>

namespace Tasking
{

/// Type to express a time in milliseconds. It can be a time point or time span.
typedef uint64_t Time; // Over 500 million years are addressable without overflow

/// Constant to specify end of time
static const Time endOfTime = std::numeric_limits<Time>::max();

/// Type to express the task ID.
typedef uint32_t TaskId;

/// Type to express the channel ID.
typedef uint32_t ChannelId;

} // namespace Tasking

#endif /* TASKTYPES_H_ */
