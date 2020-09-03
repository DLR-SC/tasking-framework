/*
 * testFifo.cpp
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

using Tasking::Fifo;

class TestFifo: public ::testing::Test {

};

TEST_F(TestFifo, testChannelId) {
  Fifo<int, 2> fifo(78u);
  EXPECT_EQ(78u, fifo.getChannelId());
  fifo.setChannelId(65u);
  EXPECT_EQ(65u, fifo.getChannelId());
}

TEST_F(TestFifo, testChannelName) {
  Fifo<int, 2> fifo("5,-$");
  EXPECT_EQ(0x352C2D24u, fifo.getChannelId());
  fifo.setChannelName("foo");
  EXPECT_EQ(0x666F6F00u, fifo.getChannelId());
  Fifo<int, 2> longNameFifo("LongName");
  EXPECT_EQ(0x4C6F6E67u, longNameFifo.getChannelId());
}

TEST_F(TestFifo, allocate) {
  Fifo<int, 2> fifo;
  EXPECT_TRUE(NULL != fifo.allocate());
  EXPECT_TRUE(NULL != fifo.allocate());
  EXPECT_TRUE(NULL == fifo.allocate());
}

TEST_F(TestFifo, dataStable) {
  Fifo<int, 2> fifo;
  int* data0 = fifo.allocate();
  int* data1 = fifo.allocate();
  *data0 = 1;
  *data1 = 1023;
  EXPECT_TRUE(1 == *data0);
  *data0 = 0xFFFF;
  EXPECT_TRUE(1023 == *data1);
}

TEST_F(TestFifo, releaseNull) {
  int* pData = NULL;
  Fifo<int, 2> fifo;
  fifo.release(pData);
  fifo.allocate(); // Should run stable
}

TEST_F(TestFifo, release0) {
  int* pData[2];
  Fifo<int, 2> fifo;
  pData[0] = fifo.allocate();
  fifo.release(pData[0]);
  fifo.allocate(); // Should run stable
}

TEST_F(TestFifo, release01) {
  int* pData[2];
  Fifo<int, 2> fifo;
  pData[0] = fifo.allocate();
  pData[1] = fifo.allocate();
  EXPECT_TRUE(pData[0] != pData[1]);
  fifo.release(pData[0]);
  fifo.release(pData[1]);
  fifo.allocate(); // Should run stable
}

TEST_F(TestFifo, release10) {
  int* pData[2];
  Fifo<int, 2> fifo;
  pData[0] = fifo.allocate();
  pData[1] = fifo.allocate();
  EXPECT_TRUE(pData[0] != pData[1]);
  fifo.release(pData[1]);
  fifo.release(pData[0]);
  fifo.allocate(); // Should run stable
}

TEST_F(TestFifo, release102) {
  int* pData[3];
  Fifo<int, 3> fifo;
  pData[0] = fifo.allocate();
  pData[1] = fifo.allocate();
  pData[2] = fifo.allocate();
  EXPECT_TRUE(pData[0] != pData[1]);
  EXPECT_TRUE(pData[0] != pData[2]);
  EXPECT_TRUE(pData[1] != pData[2]);
  fifo.release(pData[1]);
  pData[1] = fifo.allocate();
  EXPECT_TRUE(pData[0] != pData[1]);
  EXPECT_TRUE(pData[0] != pData[2]);
  EXPECT_TRUE(pData[1] != pData[2]);
  fifo.release(pData[1]);
  fifo.release(pData[0]);
  fifo.release(pData[2]);
  fifo.allocate(); // Should run stable
}

TEST_F(TestFifo, releaseWrong) {
  int wrongData;
  Fifo<int, 2> fifo;
  fifo.release(&wrongData);
  int* pInt = fifo.allocate();
  EXPECT_FALSE(pInt == &wrongData);
  fifo.release(&wrongData);
  pInt = fifo.allocate();
  EXPECT_FALSE(pInt == &wrongData);
  fifo.release(&wrongData);
  EXPECT_TRUE(NULL == fifo.allocate());
}

TEST_F(TestFifo, pushPop) {
  Fifo<int, 1> fifo;
  int* data = fifo.allocate();
  *data = 1;
  fifo.push(data);
  data = fifo.pop();
  EXPECT_TRUE(1 == *data);
  fifo.release(data);
}

TEST_F(TestFifo, Ordering) {
  Fifo<int, 5> fifo;
  for (int i = 1; i <= 5; i++) {
    int* data = fifo.allocate();
    *data = i;
    fifo.push(data);
  }
  for (int i = 1; i <= 5; i++) {
    int* data = fifo.pop();
    EXPECT_TRUE(i == *data);
  }
}

TEST_F(TestFifo, isEmpty) {
  Fifo<int, 2> fifo;
  EXPECT_TRUE(fifo.isEmpty());
  int* data = fifo.allocate();
  EXPECT_TRUE(fifo.isEmpty());
  fifo.push(data);
  EXPECT_FALSE(fifo.isEmpty());
  fifo.pop();
  EXPECT_TRUE(fifo.isEmpty());
}

TEST_F(TestFifo, pushWithoutAllocate) {
  Fifo<int, 1> fifo;
  int value = 42;
  fifo.push(&value);
  EXPECT_TRUE(fifo.isEmpty());
  EXPECT_TRUE(NULL == fifo.pop());
}

TEST_F(TestFifo, pushReference) {
  Fifo<int, 1> fifo;
  int value = 42;
  fifo.push(value);
  EXPECT_FALSE(fifo.isEmpty());
  EXPECT_FALSE(NULL == fifo.pop());
}

TEST_F(TestFifo, lastPushed) {
  Fifo<int, 2> fifo;
  int* data;
  int value = 5;
  fifo.push(value); // FIFO = [5]
//  EXPECT_EQ(5, cbuffer[1]);
  value = 42;
  fifo.push(value); // FIFO = [5, 42]
//  EXPECT_EQ(42, cbuffer[1]);
  data = fifo.pop(); // FIFO = [42]
  ASSERT_FALSE(NULL == data);
  EXPECT_EQ(5, *data);
  fifo.release(data);
  value = 111;
  fifo.push(value); // FIFO = [42, 111]
  data = fifo.pop(); // FIFO = [111]
  ASSERT_FALSE(NULL == data);
  EXPECT_EQ(42, *data);
  data = fifo.pop(); // FIFO = []
  ASSERT_FALSE(NULL == data);
  EXPECT_EQ(111, *data);
}

TEST_F(TestFifo, reuisingUnusedElements) {
  Fifo<int, 2> fifo;

  int value = 1;
  ASSERT_TRUE(fifo.push(value));
  int *data = fifo.pop();
  ASSERT_FALSE(NULL == data); // no null pointer
  EXPECT_EQ(1, *data);
  fifo.release(data);
  value = 2;
  ASSERT_TRUE(fifo.push(value));
  data = fifo.pop();
  ASSERT_FALSE(NULL == data); // no null pointer
  EXPECT_EQ(2, *data);
  fifo.release(data);
  value = 3;
  ASSERT_TRUE(fifo.push(value));
  data = fifo.pop();
  ASSERT_FALSE(NULL == data); // no null pointer
  EXPECT_EQ(3, *data);
  fifo.release(data);
  value = 4;
  ASSERT_TRUE(fifo.push(value));
  data = fifo.pop();
  ASSERT_FALSE(NULL == data); // no null pointer
  EXPECT_EQ(4, *data);
  fifo.release(data);
  value = 5;
  ASSERT_TRUE(fifo.push(value));
  data = fifo.pop();
  ASSERT_FALSE(NULL == data); // no null pointer
  EXPECT_EQ(5, *data);
  fifo.release(data);
}
