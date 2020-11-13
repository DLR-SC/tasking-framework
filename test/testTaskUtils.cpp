/*
 * testTaskUtils.cpp
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

#include <gtest/gtest.h>
#include <taskUtils.h>

// Test works only if PLATFORM is none
TEST(TestUtils, MutexGuard)
{
    class TestMutex : public Tasking::Mutex
    {
    public:
#ifdef IS_NONE_PLATFORM
        using Mutex::occupied;
#endif
    };

    TestMutex mutex;
    {
        Tasking::MutexGuard guard(mutex);
#ifdef IS_NONE_PLATFORM
        EXPECT_TRUE(mutex.occupied);
#endif
    }
#ifdef IS_NONE_PLATFORM
    EXPECT_FALSE(mutex.occupied);
#else
    EXPECT_TRUE(true);
#endif
}
