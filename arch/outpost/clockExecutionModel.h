/*
 * clockExecutionModel.h
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

#ifndef TASKING_ARCH_OUTPOST_CLOCKEXECUTIONMODEL_H_
#define TASKING_ARCH_OUTPOST_CLOCKEXECUTIONMODEL_H_

#include <impl/clock_impl.h>
#include <outpost/rtos.h>

namespace Tasking
{

/// Clock implemented on basis of the Outpost timer functionality.
class ClockExecutionModel : public Clock, public outpost::Callable
{
public:
    /**
     * Initialize clock.
     * @param scheduler Reference to the executor, for this implementation without functionality
     */
    ClockExecutionModel(Scheduler& scheduler);

    /**
     * Stop the timer
     */
    ~ClockExecutionModel(void);

    /**
     * @return Milliseconds since instantiation of the associated scheduler or since a new zero time was set.
     * @see setZeroTime
     */
    Time getTime(void) const override;

    /**
     * Method to set the zero time.
     *
     * @param offset Offset time which the zero time is adjusted. An immediate subsequent call to get
     * time will than deliver the value of this parameter.
     */
    void setZeroTime(Time offset);

protected:
    /**
     * Start the timer for a new wake up time.
     * @param timeSpan The timer after which the trigger shall start.
     */
    void startTimer(Time timeSpan) override;

private:
    /// Board clock to get the current time
    outpost::rtos::SystemClock boardClock;

    /// Zero time which indicates the time when the frame work is started
    outpost::time::SpacecraftElapsedTime m_zeroTime;

    /// The timer to drive the execution of events.
    outpost::rtos::Timer m_timer;

    /**
     * Callback function entry point for the timer.
     */
    void clockTick(outpost::rtos::Timer* timer);
};

} // namespace Tasking

#endif /* TASKING_ARCH_OUTPOST_CLOCKEXECUTIONMODEL_H_ */
