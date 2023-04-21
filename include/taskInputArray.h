/*
 * taskInputArray.h
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

#ifndef TASKINPUTARRAY_H_
#define TASKINPUTARRAY_H_

#include "impl/taskInputArray_Impl.h"

namespace Tasking
{

class Task;

/**
 * Array of inputs for a task.
 *
 * If an input array has only optional inputs, it is not triggered directly after a reset. In this case the
 * task is activated with the first incoming optional input.
 */
class InputArray
{
public:
    /**
     * Function to add a boolean function to the array to replace the default Boolean AND expression on all inputs.
     * The function can request the input states of the input array to formulate the condition to activate the
     * connected task.
     */
    using BooleanFunction = bool (*)(const InputArray&);

    /**
     * Read access to the input at the index position in the array.
     * @param index Index of the element to access.
     */
    const Input& operator[](unsigned int index) const;

    /**
     * Write read access to the input at the index position in the array.
     * @param index Index of the element to access.
     */
    Input& operator[](unsigned int index);

    /**
     * @result True when all inputs are configured. The input should be not used by a task, when the task input
     * array is not valid.
     */
    bool isValid(void) const;

    /**
     * @result number of inputs in the array.
     */
    size_t size(void) const;

    /**
     * @result True if all inputs in the array reached the activations threshold or an input with the final flag
     * reached its activations threshold.
     */
    bool isActivated(void) const;

    /**
     * Perform the reset operation on all inputs of the input array.
     */
    void reset(void);

    /**
     * Establish connection to a task for all inputs in the array
     * @param task The task the inputs are connected to
     */
    void connectTask(TaskImpl& task);

    /**
     * Set an alternative condition to replace the default AND condition of all inputs.
     * If a alternative condition is set the result of the function is used to activate the connected task.
     * Activated inputs configured as "final" will overrule the result of the alternative function.
     * @param alternativeCondition Alternative condition for the task activation.
     */
    void setCondition(BooleanFunction alternativeCondition);

protected:
    /**
     * Initialize the data of the input array. The constructor should only be used with the specialized template class,
     * which provide the memory for the array.
     * @param inputs Reference to the memory of the input array.
     * @param size Number of inputs in the array of inputs.
     */
    InputArray(Input* inputs, size_t size);

private:
    /// Implementation specific data structure with no access from application level
    InputArrayImpl impl;

    /// Pointer to the alternative condition for a task activation
    BooleanFunction condition;
};

/**
 * Array of inputs. This array provide the memory space to hold the inputs of a task.
 * @tparam n Size of the input array.
 */
template<size_t n>
class InputArrayProvider : public InputArray
{
public:
    /**
     * Initialize the memory of an input array.
     */
    InputArrayProvider(void);

protected:
    Input inputMemory[n];
};

// ----- inlines -----

template<size_t n>
InputArrayProvider<n>::InputArrayProvider(void) : InputArray(inputMemory, n)
{
}

} // namespace Tasking

#endif /* TASKINPUTARRAY_H_ */
