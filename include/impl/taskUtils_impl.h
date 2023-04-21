/*
 * taskUtils.h
 *
 * Copyright 2012-2021 German Aerospace Center (DLR) SC
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
#ifndef INCLUDE_IMPL_TASKUTILS_IMPL_H_
#define INCLUDE_IMPL_TASKUTILS_IMPL_H_

#include <type_traits>

namespace Tasking
{

/**
 * Conversion from a name to an identification.
 * @tparam IdType Identification type. It shall an unsigned integer value.
 * @param name Null terminated string with the name to convert.
 * @result Identification computed from the name.
 */
template<typename IdType>
IdType
getIdentificationFromName(const char* name)
{
    static_assert(std::is_unsigned<IdType>::value, "IdType needs to be of unsigned integer type");
    IdType id = 0u;
    if (name != nullptr && name[0] != '\0')
    {
        // Copy the new identification name. It is assumed to be null-terminated.
        size_t i = 0u;
        while ((i < sizeof(IdType)) && (name[i] != '\0'))
        {
            id = (id << 8u) + static_cast<unsigned int>(name[i]);
            ++i;
        }
        id = id << (8u * (sizeof(IdType) - i));
    }

    return id;
}

// ------------------------------------

/**
 * Conversion from an identification to a string.
 * @tparam IdType Identification type. It shall an unsigned integer value.
 * @param id Identification to convert.
 * @param buffer Pointer to buffer to hold the resulting identification name.
 * @param length Size of the buffer for task name. Optional and buffer size is assumed with five bytes.For buffer
 * with less than five bytes it is mandatory to set the value. The resulting string is shorten to the length.
 */
template<typename IdType>
char*
convertIdentificationToString(IdType id, char* buffer, size_t length)
{
    static_assert(std::is_unsigned<IdType>::value, "IdType needs unsigned integer type");
    if ((length > 1u) && (buffer != nullptr))
    {
        // Loop when we are inside the buffer length (excluding null termination) and not all bits converted
        for (size_t pos = 0u, shift = sizeof(IdType) * 8u; (pos < length - 1u) && (shift > 0u); ++pos)
        {
            shift -= 8u; // Reduce shift first, so end check on bigger than zero becomes possible
            buffer[pos] = (id >> shift) & 0xFFu;
        }
        // Minimum between length-1 and identification type size is the null termination.
        buffer[(length > sizeof(IdType) + 1u) ? sizeof(IdType) : length - 1u] = 0u;
    }
    return buffer;
}

} // namespace Tasking

#endif /* INCLUDE_IMPL_TASKUTILS_IMPL_H_ */
