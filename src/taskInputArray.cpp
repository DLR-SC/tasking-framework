/*
 * taskInputArray.cpp
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

#include <assert.h>
#include <cstddef>
#include "taskInputArray.h"

Tasking::InputArray::InputArray(Input* inputMemory, size_t p_size) : impl(inputMemory, p_size)
{
}

//-------------------------------------

const Tasking::Input&
Tasking::InputArray::operator[](unsigned int index) const
{
    assert(index < impl.length);
    return impl.inputs[index];
}

//-------------------------------------

Tasking::Input&
Tasking::InputArray::operator[](unsigned int index)
{
    assert(index < impl.length);
    return impl.inputs[index];
}

//-------------------------------------

size_t
Tasking::InputArray::size() const
{
    return impl.length;
}

//-------------------------------------

bool
Tasking::InputArray::isValid() const
{
    bool valid = true;
    for (unsigned int i = 0u; valid && (i < impl.length); ++i)
    {
        valid = valid && (impl.inputs[i].isValid());
    }
    return valid;
}

//-------------------------------------

bool
Tasking::InputArray::isActivated(void) const
{

    bool activate = true;
    bool final = false; // exit condition if an activated final input is found

    for (unsigned int i = 0; !final && (i < impl.length); i++)
    {
        if (!impl.inputs[i].isActivated())
        {
            // Change activation state only when the input is not a final and optional input
            if ((!impl.inputs[i].isFinal()) && (!impl.inputs[i].isOptional()))
            {
                activate = false; // First not activated input leads to false.
                                  // Only an active final input can change this result.
            }
        }
        else
        {
            if (impl.inputs[i].isFinal())
            {
                final = true; // if a final activated input is found, we are finished
                activate = true;
            }
        }
    }
    return activate;
}

//-------------------------------------

void
Tasking::InputArray::reset(void)
{
    for (unsigned int i = 0; i < impl.length; ++i)
    {
        impl.inputs[i].reset();
    }
}

//-------------------------------------

void
Tasking::InputArray::connectTask(TaskImpl& task)
{
    for (unsigned int i = 0u; i < impl.length; ++i)
    {
        impl.inputs[i].connectTask(task);
    }
}

// ====================================

Tasking::InputArrayImpl::InputArrayImpl(Input* inputMemory, size_t p_size) : inputs(inputMemory), length(p_size)
{
}
