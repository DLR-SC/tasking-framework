/*
 * filterExample.cpp
 * Copyright 2021 German Aerospace Center (DLR) SC
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

/*
 * In this example 3 tasks are executed. The first task generates random values
 * (from a simulated sensor) and sends them to a second task which filters it.
 * Filtered data is pushed into a second channel. Finally the last task is
 * activated only when data have been pushed into both channels, i.e. when Task
 * 1 and 2 were activated.
 *
 * Note how the code is split into framework independent and dependent code.
 */

#include <random>
#include <iostream>

#include <schedulerProvider.h>
#include <schedulePolicyFifo.h>
#include <taskChannel.h>
#include <taskEvent.h>
#include <task.h>

/*
 * Framework independent code --------------------------------------------------
 */

/*
 * Simulates a sensor producing data.
 * The data is generated according to the Normal distribution.
 */
class FakeSensor
{
public:
    explicit FakeSensor(double mean, double stdDes);
    double read();

private:
    std::random_device mRndDevice;
    std::default_random_engine mEngine;
    std::normal_distribution<> mDistribution;
};

FakeSensor::FakeSensor(double mean, double stdDes) : mEngine(mRndDevice()), mDistribution(mean, stdDes)
{
}

double
FakeSensor::read()
{
    return mDistribution(mEngine);
}

/*
 * Moving average filter of 2 samples
 */
class MovingAverageFilter
{
public:
    explicit MovingAverageFilter() = default;
    double operator()(double newValue);

private:
    bool mIsFirstValue = true;
    double mLastValue;
};

double
MovingAverageFilter::operator()(double newValue)
{
    if (mIsFirstValue)
    {
        mLastValue = newValue;
        mIsFirstValue = false;
    }
    double result = (mLastValue + newValue) * 0.5;
    mLastValue = newValue;

    return result;
}

/*
 * Prints to the standard output (just an example of consuming data)
 */
class Printer
{
public:
    explicit Printer() = default;
    void operator()(double value, double filteredValue);
};

void
Printer::operator()(double value, double filteredValue)
{
    std::cout << "Raw value: " << value << " Filtered value: " << filteredValue << std::endl;
}

/*
 * Framework dependent code ----------------------------------------------------
 */

/*
 * Alias for task
 */
template<unsigned int INPUTS>
using ApplicationTask = Tasking::TaskProvider<INPUTS, Tasking::SchedulePolicyFifo>;

/*
 * Generic channel for sharing raw and filtered data
 */
template<typename data_t>
class DataChannel : public Tasking::Channel
{
public:
    explicit DataChannel() = default;

    void
    update(data_t newValue)
    {
        mValue = newValue;
        push(); // This triggers all tasks associated to this channel
    }

    const data_t
    get() const
    {
        return mValue;
    }

private:
    data_t mValue;
};

/*
 * Application task that reads the sensor.
 * This task is triggered periodically from Tasking::Event object.
 */
class SensorTask : public ApplicationTask<1>
{
public:
    explicit SensorTask(Tasking::Scheduler& scheduler, DataChannel<double>& rawDataChannel);
    void execute(void) override;

private:
    DataChannel<double>& mRawDataChannel;
    FakeSensor mSensor;
    char mTaskName[5];
};

SensorTask::SensorTask(Tasking::Scheduler& scheduler, DataChannel<double>& rawDataChannel) :
    ApplicationTask<1>(scheduler, "SensorTask"), mRawDataChannel(rawDataChannel), mSensor(1.0, 0.2)
{
    // Configure the number of activations to 1 (the input is triggered with 1
    // push at the channel) and the input to be not final (all task should be
    // triggered for running the task)
    inputs[0].configure(1u, false);

    // Getting task name (truncated to four characters)
    Tasking::TaskId myId = getTaskId();
    Tasking::convertChannelIdToString(myId, mTaskName, sizeof(mTaskName));
}

void
SensorTask::execute(void)
{
    auto value = mSensor.read();
    mRawDataChannel.update(value);

    std::cout << "[" << mTaskName << "] executed" << std::endl;
}

/*
 * Application task that filters the raw data from the sensor.
 * This task is triggered when new data is pushed into the channel for which its
 * input was configured (see main).
 */
class FilterTask : public ApplicationTask<1>
{
public:
    explicit FilterTask(Tasking::Scheduler& scheduler, DataChannel<double>& filteredDataChannel);
    void execute(void) override;

private:
    DataChannel<double>& mFilteredDataChannel;
    MovingAverageFilter mFilter;
    char mTaskName[5];
};

FilterTask::FilterTask(Tasking::Scheduler& scheduler, DataChannel<double>& filteredDataChannel) :
    ApplicationTask<1>(scheduler, "FilterTask"), mFilteredDataChannel(filteredDataChannel)
{
    // Configure the number of activations to 1 (the input is triggered with 1
    // push at the channel) and the input to be not final (all task should be
    // triggered for running the task)
    inputs[0].configure(1u, false);

    // Getting task name (truncated to four characters)
    Tasking::TaskId myId = getTaskId();
    Tasking::convertChannelIdToString(myId, mTaskName, sizeof(mTaskName));
}

void
FilterTask::execute(void)
{
    auto& inputChannel = *getChannel<DataChannel<double>>(0u);
    auto value = inputChannel.get();
    auto filteredValue = mFilter(value);
    mFilteredDataChannel.update(filteredValue);

    std::cout << "[" << mTaskName << "] executed" << std::endl;
}

/*
 * Application task that simulates consuming the raw data and the
 * filtered data.
 * This task has 2 inputs and it is triggered only when both inputs have been
 * activated (for inputs configuration see main).
 */
class PrinterTask : public ApplicationTask<2>
{
public:
    explicit PrinterTask(Tasking::Scheduler& scheduler);
    void execute() override;

private:
    Printer mPrinter;
    char mTaskName[5];
};

PrinterTask::PrinterTask(Tasking::Scheduler& scheduler) : ApplicationTask<2>(scheduler, "PrinterTask")
{
    // Configure the number of activations to 1 (the input is triggered with 1
    // push at the channel) and the input to be not final (all task should be
    // triggered for running the task)
    inputs[0].configure(1u, false);
    inputs[1].configure(1u, false);

    // Getting task name (truncated to four characters)
    Tasking::TaskId myId = getTaskId();
    Tasking::convertChannelIdToString(myId, mTaskName, sizeof(mTaskName));
}

void
PrinterTask::execute(void)
{
    auto& rawDataChannel = *getChannel<DataChannel<double>>(0u);
    auto value = rawDataChannel.get();

    auto& filteredDataChannel = *getChannel<DataChannel<double>>(1u);
    auto filteredValue = filteredDataChannel.get();

    std::cout << "[" << mTaskName << "] ";
    mPrinter(value, filteredValue);
}

/*
 * In main, the scheduler, event, channels and tasks objects are created.
 * Additionally the input(s) of each task are configured
 */
int
main(void)
{
    Tasking::SchedulerProvider<1u, Tasking::SchedulePolicyFifo> scheduler;
    Tasking::Event event(scheduler);
    DataChannel<double> rawDataChannel;
    DataChannel<double> filteredDataChannel;

    SensorTask sensorTask(scheduler, rawDataChannel);
    sensorTask.configureInput(0, event);

    FilterTask filterTask(scheduler, filteredDataChannel);
    filterTask.configureInput(0, rawDataChannel);

    PrinterTask printerTask(scheduler);
    printerTask.configureInput(0, rawDataChannel);
    printerTask.configureInput(1, filteredDataChannel);

    // Set clock to 500ms and starts after 1s
    event.setPeriodicTiming(500, 1000u);

    // Start the scheduler with a reset action, which is needed to start timer
    scheduler.start(true);

    std::cout << "Type a line to terminate program" << std::endl;
    std::string keyBoardinput;
    std::cin >> keyBoardinput;

    // Stop the event, its safer but not really necessary for the termination
    event.stop();

    // Terminate the scheduler
    scheduler.terminate();

    return 0;
}
