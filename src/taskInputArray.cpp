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
#include "taskInputArray.h"

Tasking::InputArray::InputArray(Input* inputMemory, size_t p_size) : impl(inputMemory, p_size), condition(nullptr)
{
}

//-------------------------------------

const Tasking::Input& Tasking::InputArray::operator[](unsigned int index) const
{
    assert(index < impl.length);
    return impl.inputs[index];
}

//-------------------------------------

Tasking::Input& Tasking::InputArray::operator[](unsigned int index)
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

    bool activate;
    unsigned int lastChecked = impl.length;

    if (nullptr == condition)
    {
        // No alternative condition
        activate = true;
        for (unsigned int i = 0; i < impl.length; ++i)
        {
            if (!impl.inputs[i].isActivated())
            {
                // Change activation state only when the input is not a final and optional input
                if ((!impl.inputs[i].isFinal()) && (!impl.inputs[i].isOptional()))
                {
                    // First not activated input leads to false.
                    activate = false;
                    // Remaining inputs need a check on activated and final input
                    lastChecked = i;
                    break; // Further search is not meaningful, end loop
                }
            }
            else
            {
                if (impl.inputs[i].isFinal())
                {
                    break; // end loop over inputs if activated and final input is found.
                }
            }
        } // end for over inputs
    }
    else
    {
        // Alternative condition is set
        activate = condition(*this);
        // If not activated by condition check all inputs on final flag
        if (!activate)
        {
            lastChecked = 0u;
        }
    }

    // Check remaining inputs on final. If activated by normal condition last check is equal to length.
    for (unsigned int i = lastChecked + 1u; i < impl.length; ++i)
    {
        if (impl.inputs[i].isActivated() && impl.inputs[i].isFinal())
        {
            // Found activated final input
            activate = true;
            break;
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

//-------------------------------------

void
Tasking::InputArray::setCondition(BooleanFunction alternativeCondition)
{
    condition = alternativeCondition;
}

// ====================================

Tasking::InputArrayImpl::InputArrayImpl(Input* inputMemory, size_t p_size) : inputs(inputMemory), length(p_size)
{
}
