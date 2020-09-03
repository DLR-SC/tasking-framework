/*
 * taskInputArray_Impl.h
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

#ifndef INCLUDE_IMPL_TASKINPUTARRAY_IMPL_H_
#define INCLUDE_IMPL_TASKINPUTARRAY_IMPL_H_

#include "taskInput.h"
#include <cstddef>

namespace Tasking {

struct InputArrayImpl {
    /**
     * Initialize the data of the input array. The constructor should only used with the overloaded template class
     * which provide the memory for the array.
     * @param inputs Reference to the memory of the input array.
     * @param size Number of inputs in the array of inputs.
     */
    InputArrayImpl(Input* inputs, size_t size);

    /// Reference to the array of input pointer
    Input* inputs;
    /// Number of inputs in the array.
    size_t length;
};

}  // namespace Tasking



#endif /* INCLUDE_IMPL_TASKINPUTARRAY_IMPL_H_ */
