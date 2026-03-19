/*
 * schedulePolicyPSlot.h
 *
 * Copyright 2012-2022 German Aerospace Center (DLR) SC
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

#ifndef OTASKING_INCLUDE_SCHEDULEPOLICYPSLOT_H_
#define OTASKING_INCLUDE_SCHEDULEPOLICYPSLOT_H_

#include "schedulePolicy.h"
#include "taskUtils.h"

namespace Tasking
{

/**
 * A priority based scheduling policy using priority slots. The scheduling policy has O(1) at task enqueue and O(N) at
 * schedule time. It should be used instead SchedulePolicyPriority in case of a lower number of priorities than tasks in
 * the system.
 * @see SchedulePolicyPriority.
 */
class SchedulePolicyPSlot : public SchedulePolicy
{
protected:
    /// Structure to hold head and tail of each priority slot.
    struct FifoSlot
    {
        /// Null initialization
        explicit FifoSlot(void);

        /// Pointer to the first queued task in the FIFO or nullptr if FIFO is empty.
        TaskImpl* head;

        /// Pointer to the last queued task in the FIFO. If queue is empty, the value is not valid.
        TaskImpl* tail;
    };

public:
    /// Definition of a priority. Highest number has the highest priority.
    typedef unsigned int Priority;

    /// Initializer for the priority settings of a task.
    struct Settings
    {
        /**
         * Initialize with a priority.
         * @param priority Priority of the task. A priority higher than the number of slots is assigned to the highest
         * slot.
         */
        explicit Settings(Priority priority);

        /// Priority of the task.
        Priority priority;
    };

    /**
     * Data to manage the queued tasks in a priority slot by FIFO discipline.
     */
    struct ManagementData : public SchedulePolicy::ManagementData
    {
        /**
         * Initialize the priority of a task.
         */
        ManagementData(Settings setting);

        /// Pointer to the next task in the same priority slot. It will be scheduled after the current task.
        TaskImpl* next;

        /// Priority of the task
        Priority priority;
    };

    /**
     * Initialization scheduling policy. It is recommended to utilize the class SchedulePolicyPSlotProvider
     * @param slotMemory Pointer to the memory to hold the slots.
     * @param numberOfSlots Number of slots in the slotMemory.
     *
     * @see SchedulePolicyPSlotProvider
     */
    SchedulePolicyPSlot(FifoSlot* slotMemory, unsigned int numberOfSlots);

    /**
     * Put a task at the tail of the FIFO queue in the corresponding priority slot.
     * @param task Reference to the task, which will queued in FIFO order to the priority slot.
     * @return True when all priority slots were empty at call.
     */
    bool queue(TaskImpl& task) override;

    /**
     * Request and remove the next task from the highest priority slot.
     * @return Pointer to the head element of the FIFO in the highest non-empty priority slot at call time. If no task
     * is available in all priority slots a NULL pointer is returned.
     */
    Tasking::TaskImpl* nextTask(void) override;

protected:
    /// Pointer to the priority slots
    FifoSlot* prioritySlots;

    /// The slot with the highest prioritized slot for the next task.
    unsigned int highestPriorSlot;

    /// Number of priority slots
    unsigned int maxPrioritySlot;

    /// Mutex to protect access to priority slots
    Mutex queueMutex;
};

/**
 * Provider class to simplify setup of the schedule policy priority slot.
 * @tparam numberOfSlots Number of priority slots for the policy. This is equal to the highest priority for scheduling.
 */
template<unsigned int numberOfSlots>
class SchedulePolicyPSlotProvider : public SchedulePolicyPSlot
{
public:
    /// Initialize schedule policy priority slot.
    SchedulePolicyPSlotProvider(void);

private:
    /// Slots with FIFO queue to initialize the policy.
    SchedulePolicyPSlot::FifoSlot slots[numberOfSlots];
};

// --- implementation of provider ----

template<unsigned int numberOfSlots>
SchedulePolicyPSlotProvider<numberOfSlots>::SchedulePolicyPSlotProvider(void) :
    SchedulePolicyPSlot(slots, numberOfSlots)
{
}

} // namespace Tasking

#endif /* TASKING_INCLUDE_SCHEDULEPOLICYFIFO_H_ */
