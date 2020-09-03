/*
 * config.h
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

// Place another taskingConfig.h in the search path before to change location of the configuration.

#ifndef ARCH_OUTPOST_TASKINGCONFIG_H_
#define ARCH_OUTPOST_TASKINGCONFIG_H_

#include <stdint.h>
#include <stdlib.h>

/// Priority of an empty executor. Used priorities shall below this.
static const uint8_t executorPriority = 100u;

/// Stack size of an executor
static const size_t executorStackSize = 2048u;

#endif /* ARCH_OUTPOST_TASKINGCONFIG_H_ */
