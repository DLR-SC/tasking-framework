/*
 * iOChannelExample.cpp
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

/*
 * In this example input from the keyboard and the print out to the screen is done by special channels connected to
 * standard IO streams. By default the new line is append to the existing line, but after a time the first word would
 * disappear.
 */

#include <iostream>
#include <string>

#include <schedulerProvider.h>
#include <schedulePolicyFifo.h>
#include <taskChannel.h>
#include <taskEvent.h>
#include <task.h>

/**
 * Channel with input from the standard input.
 */
class KeyboardInputChannel : public Tasking::Channel
{
public:
    /**
     * Method which should called by main or a thread to read input on standard input to the channel.
     * The method return when 'end' is typed in.
     */
    void handleStandardInput(void);

    /// @return last read string from the input
    const std::string& getString(void) const;

protected:
    /// Last read input string from the standard input
    std::string lastInputString;
};

void
KeyboardInputChannel::handleStandardInput(void)
{

    do
    {
        // read in a line from standard input
        std::string inLine;
        std::getline(std::cin, inLine);
        lastInputString = inLine;
        // bublish the line. This trigger all tasks associated to this channel
        push();
    } while (lastInputString != "end"); // end is used to stop program
}

const std::string&
KeyboardInputChannel::getString(void) const
{
    return lastInputString;
}

// --------------------------

/// An channel connected to the standard output.
class OutputChannel : public Tasking::Channel
{
public:
    /**
     * Print out a line to standard output and save the data
     */
    void print(std::string& line);

    /// @return The last written line
    const std::string& getLastWrittenLine(void) const;

protected:
    std::string lastLine;
};

void
OutputChannel::print(std::string& line)
{
    // Save the line which is printed
    lastLine = line;
    // Write line to standard output
    std::cout << line << std::endl;
}

const std::string&
OutputChannel::getLastWrittenLine(void) const
{
    return lastLine;
}

// --------------------------

/// Read in the input data und push it to the outgoing channel
class HandleKeyboardInput : public Tasking::TaskProvider<1u, Tasking::SchedulePolicyFifo>
{
public:
    HandleKeyboardInput(Tasking::Scheduler& scheduler, OutputChannel& outChannel, Tasking::Event& outTime);
    virtual void execute(void);

private:
    OutputChannel& out;
    Tasking::Event& outTrigger;
};

HandleKeyboardInput::HandleKeyboardInput(Tasking::Scheduler& scheduler, OutputChannel& outChannel,
                                         Tasking::Event& outTime) :
    TaskProvider(scheduler),
    out(outChannel),
    outTrigger(outTime)
{
    // Trigger when data is received on the input channel
    inputs[0u].configure(1u);
}

void
HandleKeyboardInput::execute(void)
{
    // Attach the new input line to the last written line ...
    std::string newLine =
            out.getLastWrittenLine() + std::string(" ") + getChannel<KeyboardInputChannel>(0u)->getString();
    // ... and print out
    out.print(newLine);
    // Wait 5 secondes to start removing first word from output line
    outTrigger.trigger(5000u);
}

// --------------------------

/// Remove first word from last printed line and print out
class ModifiyLastWrittenLine : public Tasking::TaskProvider<1u, Tasking::SchedulePolicyFifo>
{
public:
    ModifiyLastWrittenLine(Tasking::Scheduler& scheduler, OutputChannel& outChannel);
    virtual void execute(void);

private:
    OutputChannel& out;
};

ModifiyLastWrittenLine::ModifiyLastWrittenLine(Tasking::Scheduler& scheduler, OutputChannel& outChannel) :
    TaskProvider(scheduler),
    out(outChannel)
{
    // Start when it is trigger on input zero
    inputs[0u].configure(1u);
}

void
ModifiyLastWrittenLine::execute(void)
{
    // Remove first word from last written line so long the line is not empty
    std::string newLine = out.getLastWrittenLine();
    if (newLine.size() > 0)
    {
        // Find end of first word
        size_t pos = newLine.find(' ');
        // Remove the word or clear the line when there are no further words in line
        if (pos < newLine.size())
        {
            newLine = newLine.substr(pos + 1);
        }
        else
        {
            newLine.clear();
        }
        // Output the new line
        out.print(newLine);
        // Self trigger in three seconds to remove the next word from line
        getChannel<Tasking::Event>(0u)->trigger(3000);
    }
}
// <<<<<<== instances ==>>>>>

/// Instantiate the input channel
Tasking::SchedulerProvider<1u, Tasking::SchedulePolicyFifo> scheduler;
KeyboardInputChannel inChannel;
OutputChannel outChannel;
Tasking::Event modifyTrigger(scheduler);
HandleKeyboardInput handleInput(scheduler, outChannel, modifyTrigger);
ModifiyLastWrittenLine modifyTask(scheduler, outChannel);

// <<<<<< == program code == >>>>>

int
main(void)
{
    std::cout << "Type in a sentence. If you type in within five seconds a further sentence, this sentende is "
              << std::endl;
    std::cout << "Attached to the first sentence. When the five seconds are over the first word is removed."
              << std::endl;
    std::cout << "So long no further sentence is typed in within three seconds the next first word is removed."
              << std::endl;
    std::cout << std::endl;
    std::cout << "Type in end as single word to stop the program." << std::endl;

    // Connect tasks to channels
    handleInput.configureInput(0u, inChannel);
    modifyTask.configureInput(0u, modifyTrigger);

    // Start Tasking scheduler
    scheduler.start();

    // Use main as thread for inputs. Tasks should not block
    inChannel.handleStandardInput();

    // Stop Tasking scheduler
    scheduler.terminate(true);

    return 0;
}
