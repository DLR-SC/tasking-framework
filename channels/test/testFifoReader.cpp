/*
 * testFifoReader.cpp
 *
 * Copyright 2019 German Aerospace Center (DLR) SC
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

#include <stdint.h>

#include "gtest/gtest.h"
#include <channels/fifo.h>
#include <taskInput.h>
#include <task.h>
#include <schedulerUnitTest.h>
#include <schedulePolicyLifo.h>

class TestFifoReader: public ::testing::Test {
  public:

    static const int fifoSize = 7;

    class TestTask: public Tasking::TaskProvider<1, Tasking::SchedulePolicyLifo> {
    public:
        TestTask(Tasking::Scheduler& p_scheduler): TaskProvider<1,Tasking::SchedulePolicyLifo>(p_scheduler) {
        }
        void execute(void) override {
        }
    };

    // Make protected stuff public
    class TestFifo: public Tasking::Fifo<int8_t, fifoSize> {
    public:
        using Tasking::Fifo<int8_t, fifoSize>::synchronizeStart;
        using Tasking::Fifo<int8_t, fifoSize>::synchronizeEnd;
    };

    Tasking::SchedulePolicyLifo policy;
    Tasking::SchedulerUnitTest scheduler;
    TestFifo fifo;
    Tasking::InputArrayProvider<3> inputs;
    Tasking::FifoReader<int8_t> reader1;
    TestTask task1;
    Tasking::FifoReader<int8_t> reader2;
    TestTask task2;
    Tasking::FifoReader<int8_t> reader3;
    TestTask task3;

    TestFifoReader(void) :
        scheduler(policy), //
	fifo(), //
	reader1(&task1), task1(scheduler), //
	reader2(&task2), task2(scheduler), //
	reader3(&task3), task3(scheduler) {
      task1.configureInput(0, fifo);
      task2.configureInput(0, fifo);
      task3.configureInput(0, fifo);
      // For the test the scheduler shall not start.
    }
};

TEST_F(TestFifoReader, OneReaderIsEmpty) {
  fifo.associateReader(reader1);
  EXPECT_TRUE(reader1.isEmpty());
  int8_t value = 42;
  fifo.push(value);
  EXPECT_TRUE(reader1.isEmpty());
  fifo.synchronizeStart(&task1, 1);
  EXPECT_FALSE(reader1.isEmpty());
  reader1.pop();
  EXPECT_TRUE(reader1.isEmpty());
}

TEST_F(TestFifoReader, OneReaderSevenMessages) {
  fifo.associateReader(reader1);
  EXPECT_TRUE(reader1.isEmpty());
  int8_t val1 = 1;
  fifo.push(val1);
  int8_t val2 = 2;
  fifo.push(val2);
  int8_t val3 = 3;
  fifo.push(val3);
  int8_t val4 = 4;
  fifo.push(val4);
  int8_t val5 = 5;
  fifo.push(val5);
  int8_t val6 = 6;
  fifo.push(val6);
  int8_t val7 = 7;
  fifo.push(val7);
  EXPECT_TRUE(reader1.isEmpty());
  fifo.synchronizeStart(&task1, 7);
  EXPECT_FALSE(reader1.isEmpty());
  EXPECT_EQ(1, *reader1.pop());
  EXPECT_EQ(2, *reader1.pop());
  EXPECT_EQ(3, *reader1.pop());
  EXPECT_EQ(4, *reader1.pop());
  EXPECT_EQ(5, *reader1.pop());
  EXPECT_FALSE(reader1.isEmpty());
  int8_t* returnvalue = reader1.pop();
  ASSERT_TRUE(returnvalue); // check for null pointer
  EXPECT_EQ(6, *returnvalue);
  EXPECT_FALSE(reader1.isEmpty());
  returnvalue = reader1.pop();
  ASSERT_TRUE(returnvalue); // check for null pointer
  EXPECT_EQ(7, *returnvalue);
  EXPECT_TRUE(reader1.isEmpty());
}

TEST_F(TestFifoReader, OneReaderFullCycle) {
  fifo.associateReader(reader1);
  int8_t value = 41;
  fifo.push(value);
  EXPECT_TRUE(NULL == reader1.pop());
  fifo.synchronizeStart(&task1, 1);
  int8_t* element = reader1.pop();
  ASSERT_FALSE(NULL == element);
  EXPECT_TRUE(value == *element);
  reader1.release(element);
  EXPECT_TRUE(reader1.isEmpty());
}

TEST_F(TestFifoReader, OneReaderOrdering) {
  fifo.associateReader(reader1);
  for (int i = 1; i <= fifoSize; i++) {
    fifo.push(i);
  }
  fifo.synchronizeStart(&task1, fifoSize);
  for (int i = 1; i <= fifoSize; i++) {
    int8_t* data = reader1.pop();
    ASSERT_FALSE(NULL == data);
    EXPECT_TRUE(i == *data);
    reader1.release(data);
  }
}

TEST_F(TestFifoReader, OneReaderNoPush) {
  fifo.associateReader(reader1);
  fifo.synchronizeStart(&task1, 0);
  EXPECT_TRUE(reader1.isEmpty());
  int8_t* element = reader1.pop();
  EXPECT_TRUE(NULL == element);
}

TEST_F(TestFifoReader, OneReaderNoFullRead) {
  fifo.associateReader(reader1);
  for (int i = 1; i <= fifoSize; i++) {
    fifo.push(i);
  }
  fifo.synchronizeStart(&task1, fifoSize);
  for (int i = 1; i <= 2; i++) {
    int8_t* data = reader1.pop();
    ASSERT_FALSE(NULL == data);
    EXPECT_TRUE(i == *data);
    reader1.release(data);
  }
  fifo.synchronizeEnd(&task1);
  task1.reset();
  fifo.push(fifoSize + 1);
  fifo.synchronizeStart(&task1, 1);
  EXPECT_FALSE(reader1.isEmpty());
  for (int i = 3; i <= fifoSize + 1; i++) {
    int8_t* data = reader1.pop();
    ASSERT_FALSE(NULL == data);
    EXPECT_TRUE(i == *data);
    reader1.release(data);
  }
  EXPECT_TRUE(reader1.isEmpty());
}

TEST_F(TestFifoReader, ArrivalDuringRead) {
  fifo.associateReader(reader1);
  fifo.push(1);
  fifo.push(2);
  fifo.synchronizeStart(&task1, 2);
  int8_t* data = reader1.pop();
  fifo.push(3);
  reader1.release(data);
  data = reader1.pop();
  reader1.release(data);
  EXPECT_TRUE(reader1.isEmpty());
  fifo.synchronizeEnd(&task1);
  task1.reset();
  fifo.synchronizeStart(&task1, 1);
  EXPECT_FALSE(reader1.isEmpty());
  data = reader1.pop();
  ASSERT_FALSE(data == NULL);
  EXPECT_TRUE(3 == *data);
  EXPECT_TRUE(reader1.isEmpty());
}

TEST_F(TestFifoReader, OrderingTwoReaderSequence) {
  fifo.associateReader(reader1);
  fifo.associateReader(reader2);
  for (int i = 1; i <= fifoSize; i++) {
    fifo.push(i);
  }
  fifo.synchronizeStart(&task1, fifoSize);
  for (int i = 1; i <= fifoSize; i++) {
    int8_t* data = reader1.pop();
    ASSERT_FALSE(NULL == data);
    EXPECT_TRUE(i == *data);
    reader1.release(data);
  }
  fifo.synchronizeEnd(&task1);
  task1.reset();
  fifo.synchronizeStart(&task2, fifoSize);
  for (int i = 1; i <= fifoSize; i++) {
    int8_t* data = reader2.pop();
    ASSERT_FALSE(NULL == data);
    EXPECT_TRUE(i == *data);
    reader2.release(data);
  }
}

TEST_F(TestFifoReader, OrderingTwoReaderDifferentAmount) {
  fifo.associateReader(reader1);
  fifo.associateReader(reader2);
  for (int i = 1; i <= 3; i++) {
    fifo.push(i);
  }
  fifo.synchronizeStart(&task2, 3);
  for (int i = 1; i <= 3; i++) {
    int8_t* data = reader2.pop();
    ASSERT_FALSE(NULL == data);
    EXPECT_TRUE(i == *data);
    reader2.release(data);
  }
  fifo.synchronizeEnd(&task2);
  task2.reset();
  EXPECT_TRUE(reader1.isEmpty());
  EXPECT_TRUE(reader2.isEmpty());
  for (int i = 4; i <= fifoSize; i++) {
    fifo.push(i);
  }
  fifo.synchronizeStart(&task1, fifoSize);
  for (int i = 1; i <= fifoSize; i++) {
    int8_t* data = reader1.pop();
    ASSERT_FALSE(NULL == data);
    EXPECT_TRUE(i == *data);
    reader1.release(data);
  }
  fifo.synchronizeEnd(&task1);
  task1.reset();
  EXPECT_TRUE(reader1.isEmpty());
  fifo.synchronizeStart(&task2, fifoSize - 3);
  for (int i = 4; i <= fifoSize; i++) {
    int8_t* data = reader2.pop();
    ASSERT_FALSE(NULL == data);
    EXPECT_TRUE(i == *data);
    reader2.release(data);
  }
  EXPECT_TRUE(reader2.isEmpty());
}

TEST_F(TestFifoReader, OrderingTwoReaderParallel) {
  fifo.associateReader(reader1);
  fifo.associateReader(reader2);
  for (int i = 1; i <= fifoSize; i++) {
    fifo.push(i);
  }
  fifo.synchronizeStart(&task1, fifoSize);
  fifo.synchronizeStart(&task2, fifoSize);
  for (int i = 1; i <= 2; i++) {
    int8_t* data = reader1.pop();
    ASSERT_FALSE(NULL == data);
    EXPECT_TRUE(i == *data);
    reader1.release(data);
  }
  for (int i = 1; i <= 3; i++) {
    int8_t* data = reader2.pop();
    ASSERT_FALSE(NULL == data);
    EXPECT_TRUE(i == *data);
    reader2.release(data);
  }
  for (int i = 3; i <= fifoSize; i++) {
    int8_t* data = reader1.pop();
    ASSERT_FALSE(NULL == data);
    EXPECT_TRUE(i == *data);
    reader1.release(data);
  }
  for (int i = 4; i <= fifoSize; i++) {
    int8_t* data = reader2.pop();
    ASSERT_FALSE(NULL == data);
    EXPECT_TRUE(i == *data);
    reader2.release(data);
  }
}

TEST_F(TestFifoReader, OrderingTwoReaderSequentially) {
  fifo.associateReader(reader1);
  fifo.associateReader(reader2);
  for (int i = 1; i <= fifoSize; i++) {
    fifo.push(i);
  }
  fifo.synchronizeStart(&task1, fifoSize);
  for (int i = 1; i <= fifoSize; i++) {
    int8_t* data = reader1.pop();
    ASSERT_FALSE(NULL == data);
    EXPECT_EQ(i, *data);
    reader1.release(data);
  }
  fifo.synchronizeEnd(&task1);
  task1.reset();
  task2.reset();
  fifo.synchronizeStart(&task2, fifoSize);
  for (int i = 1; i <= fifoSize; i++) {
    int8_t* data = reader2.pop();
    ASSERT_FALSE(NULL == data);
    EXPECT_EQ(i, *data);
    reader2.release(data);
  }
  fifo.synchronizeEnd(&task2);
}

TEST_F(TestFifoReader, OrderingTwoReaderSequentiallyWithFillingInBetween) {
  fifo.associateReader(reader1);
  fifo.associateReader(reader2);
  for (int i = 1; i <= 3; i++) {
    fifo.push(i);
  }
  fifo.synchronizeStart(&task1, 3);
  for (int i = 1; i <= 3; i++) {
    int8_t* data = reader1.pop();
    ASSERT_FALSE(NULL == data);
    EXPECT_EQ(i, *data);
    reader1.release(data);
  }
  fifo.synchronizeEnd(&task1);
  for (int i = 4; i <= fifoSize; i++) {
    fifo.push(i);
  }
  task1.reset();
  task2.reset();
  fifo.synchronizeStart(&task1, fifoSize - 3);
  for (int i = 4; i <= fifoSize; i++) {
    int8_t* data = reader1.pop();
    ASSERT_FALSE(NULL == data);
    EXPECT_EQ(i, *data);
    reader1.release(data);
  }
  fifo.synchronizeEnd(&task1);
  fifo.synchronizeStart(&task2, fifoSize);
  for (int i = 1; i <= fifoSize; i++) {
    int8_t* data = reader2.pop();
    ASSERT_FALSE(NULL == data);
    EXPECT_EQ(i, *data);
    reader2.release(data);
  }
  fifo.synchronizeEnd(&task2);
}

TEST_F(TestFifoReader, ReleaseFirstReader) {
  fifo.associateReader(reader1);
  fifo.associateReader(reader2);
  fifo.push(1);
  fifo.synchronizeStart(&task1, 1);
  int8_t* data = reader1.pop();
  ASSERT_FALSE(NULL == data);
  EXPECT_TRUE(1 == *data);
  reader1.release(data);
  fifo.synchronizeEnd(&task1);
  task1.reset();
  fifo.synchronizeStart(&task2, 1);
  data = reader2.pop();
  ASSERT_FALSE(NULL == data);
  EXPECT_TRUE(1 == *data);
  reader2.release(data);
  fifo.synchronizeEnd(&task2);
  task2.reset();
  fifo.releaseReader(reader1);
  fifo.push(2);
  fifo.synchronizeStart(&task1, 1);
  EXPECT_TRUE(reader1.isEmpty());
  fifo.synchronizeStart(&task2, 1);
  data = reader2.pop();
  ASSERT_FALSE(NULL == data);
  EXPECT_TRUE(2 == *data);
}

TEST_F(TestFifoReader, ReleaseSecondReader) {
  fifo.associateReader(reader1);
  fifo.associateReader(reader2);
  fifo.push(1);
  fifo.synchronizeStart(&task1, 1);
  int8_t* data = reader1.pop();
  ASSERT_FALSE(NULL == data);
  EXPECT_TRUE(1 == *data);
  reader1.release(data);
  fifo.synchronizeEnd(&task1);
  task1.reset();
  fifo.synchronizeStart(&task2, 1);
  data = reader2.pop();
  ASSERT_FALSE(NULL == data);
  EXPECT_TRUE(1 == *data);
  reader2.release(data);
  fifo.synchronizeEnd(&task2);
  task2.reset();
  fifo.releaseReader(reader2);
  fifo.push(2);
  fifo.synchronizeStart(&task1, 1);
  data = reader1.pop();
  ASSERT_FALSE(NULL == data);
  EXPECT_TRUE(2 == *data);
  fifo.synchronizeEnd(&task1);
  task1.reset();
  fifo.synchronizeStart(&task2, 1);
  EXPECT_TRUE(reader2.isEmpty());
}

TEST_F(TestFifoReader, ReleaseMiddleReader) {
  fifo.associateReader(reader1);
  fifo.associateReader(reader3);
  fifo.associateReader(reader2);
  fifo.push(1);
  fifo.synchronizeStart(&task1, 1);
  int8_t* data = reader1.pop();
  reader1.release(data);
  fifo.synchronizeEnd(&task1);
  task1.reset();
  fifo.synchronizeStart(&task2, 1);
  fifo.synchronizeStart(&task3, 1);
  data = reader2.pop();
  reader2.release(data);
  data = reader3.pop();
  reader3.release(data);
  fifo.synchronizeEnd(&task2);
  task2.reset();
  fifo.synchronizeEnd(&task3);
  task3.reset();
  fifo.releaseReader(reader2);
  fifo.push(2);
  fifo.synchronizeStart(&task1, 1);
  data = reader1.pop();
  ASSERT_FALSE(NULL == data);
  EXPECT_TRUE(2 == *data);
  EXPECT_TRUE(reader1.isEmpty());
  EXPECT_TRUE(reader2.isEmpty());
  EXPECT_TRUE(reader3.isEmpty());
  fifo.synchronizeEnd(&task1);
  task1.reset();
  reader1.release(data);
  fifo.synchronizeStart(&task2, 1);
  data = reader2.pop();
  EXPECT_TRUE(NULL == data);
  fifo.synchronizeEnd(&task2);
  task2.reset();
  fifo.synchronizeStart(&task3, 1);
  data = reader3.pop();
  ASSERT_FALSE(NULL == data);
  EXPECT_TRUE(2 == *data);
}

TEST_F(TestFifoReader, RemoveAllReaders) {
  fifo.associateReader(reader1);
  fifo.associateReader(reader2);
  fifo.associateReader(reader3);
  fifo.releaseReader(reader1);
  fifo.releaseReader(reader2);
  fifo.releaseReader(reader3);
  fifo.push(45);
  EXPECT_FALSE(fifo.isEmpty());
  int8_t* data = fifo.pop();
  ASSERT_FALSE(NULL == data);
  EXPECT_TRUE(45 == *data);
}

TEST_F(TestFifoReader, RemoveOneReaderNotAllRead) {
  fifo.associateReader(reader1);
  fifo.associateReader(reader2);
  fifo.push(1);
  fifo.releaseReader(reader1);
  fifo.synchronizeStart(&task2, 1);
  int8_t* data = reader2.pop();
  reader2.release(data);
  fifo.synchronizeEnd(&task2);
  task2.reset();
  // Expect FIFO is now empty, so FIFO size elements are fit
  for (int i = 2; i <= fifoSize + 1; i++) {
    fifo.push(i);
  }
  fifo.synchronizeStart(&task2, fifoSize);
  for (int i = 2; i <= fifoSize; i++) {
    data = reader2.pop();
  }
  EXPECT_FALSE(reader2.isEmpty());
  data = reader2.pop(); // fifoSize + 1
  ASSERT_FALSE(NULL == data);
  EXPECT_EQ(fifoSize + 1, *data);
  EXPECT_TRUE(reader2.isEmpty());
}

TEST_F(TestFifoReader, ReaderNoConsumeAfterAssociate) {
  fifo.associateReader(reader1);
  fifo.push(1);
  fifo.associateReader(reader2); // Shall not consume the old element
  fifo.push(2);
  fifo.synchronizeStart(&task2, 2);
  int8_t* data;
  for (int i = 1; i <= 2; i++) {
    data = reader2.pop();
    ASSERT_FALSE(data == NULL);
    EXPECT_TRUE(i == *data);
    reader2.release(data);
  }
  fifo.synchronizeEnd(&task2);
  task2.reset();
  fifo.synchronizeStart(&task1, 2);
  for (int i = 1; i <= 2; i++) {
    data = reader1.pop();
    ASSERT_FALSE(data == NULL);
    EXPECT_TRUE(i == *data);
  }
}

TEST_F(TestFifoReader, ReaderReleaseShuffled) {
    fifo.associateReader(reader1);
    // Push seven some data
    ASSERT_TRUE(fifo.push(1));
    ASSERT_TRUE(fifo.push(2));
    ASSERT_TRUE(fifo.push(3));

    fifo.synchronizeStart(&task1, 3);

    // Pop the data and save it temporarily
    int8_t* temp1 = reader1.pop();
    EXPECT_EQ(1, *temp1);
    int8_t* temp2 = reader1.pop();
    EXPECT_EQ(2, *temp2);
    int8_t* temp3 = reader1.pop();
    EXPECT_EQ(3, *temp3);

    // Release the element containing 2
    reader1.release(temp2);
    // Allocate data again --> should be the element containing 2
    temp2 = fifo.allocate();
    EXPECT_EQ(2, *temp2);
}
