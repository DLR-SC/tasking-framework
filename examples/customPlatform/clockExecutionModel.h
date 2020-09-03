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

#ifndef TASKING_EXAMPLE_CUSTOM_CLOCKEXECUTIONMODEL_H_
#define TASKING_EXAMPLE_CUSTOM_CLOCKEXECUTIONMODEL_H_

#include <impl/clock_impl.h>

namespace Tasking
{

/// Implementation of a clock without any functionality
class ClockExecutionModel : public Clock
{
public:
    /**
     * Initialize clock.
     * @param scheduler Reference to the executor, for this implementation without functionality
     */
    ClockExecutionModel(Scheduler& scheduler);

    /// @return Always time point zero for this clock
    Time getTime(void) const override;

    /**
     * Method to set the zero time.
     *
     * @param offset Offset time to which the zero time is adjusted. An immediate subsequent call to get
     * time will be adjusted by the offset starting from zero.
     */
    void setZeroTime(Time offset);

    /**
     * Forward the current time as simulation
     */
    void tick(void);

protected:
    /**
     * Do not start a timer
     */
    void startTimer(Time) override;

private:
    /// Zero time, which can adjusted by setZeroTime
    Time zeroTime;

    /// Current simulated time
    Time currentTime;
};

} // namespace Tasking

#endif /* TASKING_EXAMPLE_CUSTOM_CLOCKEXECUTIONMODEL_H_ */
