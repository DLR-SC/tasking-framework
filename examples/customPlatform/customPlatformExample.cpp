/*
 * customPlatformExample.cpp
 *
 *  Created on: 22.04.2020
 *      Author: omai
 */

#include <iostream>
#include <string>

#include <schedulerProvider.h>
#include <schedulePolicyFifo.h>
#include <task.h>
#include <taskChannel.h>

class EchoChannel : public Tasking::Channel
{
public:
    /**
     * Push sentence to channel
     */
    void push(const std::string& sentence);

    /// Read the sentence stored in the channel
    const std::string& getString(void) const;

    using Channel::reset;

protected:
    /// Container string
    std::string data;

}; // EchoChannel

/**
 * A Task with two incoming channels and one outgoing channel.
 * The Task concatenate the strings from the incoming channels, print it out and push them to the outgoing channel.
 */
class EchoTask : public Tasking::TaskProvider<2, Tasking::SchedulePolicyFifo>
{
public:
    /**
     * Configure inputs with one activation on each and initialize task.
     */
    EchoTask(Tasking::Scheduler& scheduler, std::string concatWord, EchoChannel& outChannel);

    /// Output and forward constructed words from the incoming channels.
    void execute(void) override;

protected:
    /// String between words from incoming channels
    std::string concatenator;

    /// Outgoing channel for resulting sentences
    EchoChannel& out;

}; // EchoTask

// ==========================

void
EchoChannel::push(const std::string& sentence)
{
    data = sentence;
    Channel::push();
}

// --------------------------

const std::string&
EchoChannel::getString(void) const
{
    return data;
}

// --------------------------

EchoTask::EchoTask(Tasking::Scheduler& scheduler, std::string concatWord, EchoChannel& outChannel) :
    TaskProvider(scheduler),
    out(outChannel)
{
    // Activate when one new string is available on both incoming channels.
    inputs[0].configure(1u);
    inputs[1].configure(1u);
    // Construct concatenator string
    concatenator = std::string(" ") + concatWord + std::string(" ");

} // EchoTask::EchoTask(Tasking::Scheduler&, std::string, EchoChannel&)

// --------------------------

void
EchoTask::execute(void)
{
    std::string newSentence = getChannel<EchoChannel>(0u)->getString();
    const std::string& appendix = getChannel<EchoChannel>(1u)->getString();
    if (appendix.length() > 0u)
    {
        newSentence += concatenator + appendix;
    }
    std::cout << newSentence << std::endl;
    out.push(newSentence);

} // EchoTask::execute()

// ==========================
// Construct tasks and channels
Tasking::SchedulerProvider<1u, Tasking::SchedulePolicyFifo> scheduler;

EchoChannel mainChannel0;
EchoChannel mainChannel1;
EchoChannel intermediateChannel12;
EchoChannel intermediateChannel23;
EchoChannel sink;

EchoTask task1(scheduler, "or", intermediateChannel12);
EchoTask task2(scheduler, "with", intermediateChannel23);
EchoTask task3(scheduler, "and", sink);

// ==========================

int
main(int argc, char* argv[])
{
    if (argc == 1)
    {
        std::cerr << "Usage: " << std::string(argv[0]) << " <word>*" << std::endl;
    }

    // Connect tasks and channels
    task1.configureInput(0, mainChannel0);
    task1.configureInput(1, mainChannel1);
    task2.configureInput(0, intermediateChannel12);
    task2.configureInput(1, mainChannel0);
    task3.configureInput(1, intermediateChannel23);
    task3.configureInput(0, mainChannel1);

    // Initialize tasks and channels before Tasking is started.
    scheduler.initialize();
    scheduler.start(true);

    mainChannel0.push("");
    // Do not use this push as trigger.
    mainChannel0.reset();

    // Loop over all words in arguments and drive process
    for (int i = 1; i < argc; ++i)
    {
        // Fill channels at the begin of the sequence
        mainChannel1.push(mainChannel0.getString());
        mainChannel0.push(argv[i]);
        // Execute pending tasks
        scheduler.step();
    } // for over argv

    // Terminate with execution of all pending tasks.
    scheduler.terminate(true);

    return 0;
} // main()
