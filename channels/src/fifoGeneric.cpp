/*
 * fifoGeneric.cpp
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

#include <channels/fifoGeneric.h>

#include <assert.h>

using Tasking::FifoGeneric;
using Tasking::FifoGenericReader;

FifoGenericReader::FifoGenericReader(Tasking::Task* p_task) :
    nextReader(nullptr),
    readerId(0),
    readerTask(p_task),
    fifo(nullptr),
    fifo_first(nullptr),
    fifo_last(nullptr),
    allocated_elements(nullptr),
    unusedLinks(nullptr)
{

    // Check if list of unused links is initialized and initialize it if necessary.
    linkMutex.enter();
    if (unusedLinks == nullptr)
    {
        for (unsigned int i = 0; i < maximumFifoReaderLinks - 1; i++)
        {
            links[i].next = &(links[i + 1]);
        }
        links[maximumFifoReaderLinks - 1].next = nullptr;
        unusedLinks = links;
    }
    linkMutex.leave();
}

// -------------------

FifoGenericReader::~FifoGenericReader(void)
{
    releaseAll();
}

// -------------------

bool
FifoGenericReader::isEmpty(void) const
{
    return (fifo_first == nullptr);
}

// -------------------

void
FifoGenericReader::release(void* data)
{
    // If not associated to a FIFO or no elements are popped, than do nothing.
    if ((fifo == nullptr) || (allocated_elements == nullptr))
    {
        return;
    }
    if (allocated_elements->fifoElement->data == data)
    {
        // Head of allocated elements
        fifo->releaseElement(allocated_elements->fifoElement, readerId);
        linkMutex.enter();
        Chain* next = allocated_elements->next;
        allocated_elements->next = unusedLinks;
        unusedLinks = allocated_elements;
        allocated_elements = next;
        linkMutex.leave();
    }
    else
    { // Not head of allocated elements
        // Find the data in the list of allocated elements
        Chain* previous = allocated_elements;
        while ((previous->next != nullptr) && (previous->next->fifoElement->data != data))
        {
            previous = previous->next;
        }
        if (previous->next != nullptr)
        {
            fifo->releaseElement(previous->next->fifoElement, readerId);
            // Hang out next link and put it to unused links
            Chain* newNext = previous->next->next;
            linkMutex.enter();
            previous->next->next = unusedLinks;
            unusedLinks = previous->next;
            linkMutex.leave();
            previous->next = newNext;
        }
    }
}

// -------------------

void*
FifoGenericReader::pop(void)
{
    if ((fifo == nullptr) || (fifo_first == nullptr))
    {
        return nullptr;
    }
    // Get link to manage associations.
    linkMutex.enter();
    Chain* link = unusedLinks;
    if (unusedLinks != nullptr)
    {
        unusedLinks = unusedLinks->next;
    }
    linkMutex.leave();
    if (link == nullptr)
    {
        // No link is available, pop isn't possible
        return nullptr;
    }
    // The result is always the data of the first FIFO element
    link->fifoElement = fifo_first;
    // Hang in the list of allocated elements
    link->next = allocated_elements;
    allocated_elements = link;
    // A pop on an empty list shall return a null pointer instead crashing.
    // Remove first element from the list
    fifo_first = link->fifoElement->next;
    // If the result is the last one during synchronize of the FIFO the reader becomes empty.
    if (link->fifoElement == fifo_last)
    {
        fifo_first = nullptr;
        fifo_last = nullptr;
    }
    else
    {
        assert(fifo_first != nullptr);
    }
    return link->fifoElement->data;
}
// -------------------

void
FifoGenericReader::releaseAll(void)
{
    // Loop over allocated elements and release them in FIFO
    linkMutex.enter();
    while (allocated_elements != nullptr)
    {
        fifo->releaseElement(allocated_elements->fifoElement, readerId);
        Chain* next = allocated_elements->next;
        allocated_elements->next = unusedLinks;
        unusedLinks = allocated_elements;
        allocated_elements = next;
    }
    linkMutex.leave();
}

// -------------------

FifoGeneric::FifoGeneric(FifoGeneric::Chain* chain, void* dataBuffer, const size_t size, const unsigned int items) :
    m_unused(nullptr),
    m_allocated(nullptr),
    m_fifo_first(nullptr),
    m_fifo_last(nullptr),
    m_lastPushed(nullptr),
    m_readers(nullptr),
    m_readerBitMask(1 << 0)
{

    char* cDataBuffer = reinterpret_cast<char*>(dataBuffer);
    for (unsigned int i = 0; i < items; i++)
    {
        chain[i].data = cDataBuffer + (size * i);
        chain[i].next = m_unused;
        m_unused = chain + i;
    }
}
// -------------------

void*
FifoGeneric::allocate(void)
{
    void* result = nullptr;
    m_mutex.enter();
    // By default the result is provided by the head of the unused list.
    Chain* element = m_unused;
    if (m_unused != nullptr)
    {
        // Remove from unused list and insert in allocated list.
        m_unused = m_unused->next;
        element->next = m_allocated;
        element->expectedReadsBitMask = (1 << 0);
        m_allocated = element;
        // Result is the corresponding data element.
        result = element->data;
    }
    m_mutex.leave();
    return result;
}

// -------------------

void
FifoGeneric::release(const void* data, unsigned int readerId)
{
    // Is there at least one allocated data item?
    if (m_allocated != nullptr)
    {
        m_mutex.enter(); // Should synchronize with other access to object
        // Is data item in the head element of allocated list?
        if (m_allocated->data == data)
        {
            // Release data
            Chain* removeItem = m_allocated;
            removeItem->expectedReadsBitMask &= ~readerId;
            if (removeItem->expectedReadsBitMask == 0)
            {
                m_allocated = removeItem->next;
                removeItem->next = m_unused;
                m_unused = removeItem;
            }
        }
        else
        {
            // Search data item in list
            Chain* previous = findPrevious(data);
            // Have we found the data item, than release it by remove it from allocate list and put it to unused list
            if (previous != nullptr)
            {
                Chain* removeItem = previous->next;
                removeItem->expectedReadsBitMask &= ~readerId;
                if (removeItem->expectedReadsBitMask == 0)
                {
                    previous->next = removeItem->next;
                    removeItem->next = m_unused;
                    m_unused = removeItem;
                }
            }
        }
        m_mutex.leave();
    }
}

// -------------------

void
FifoGeneric::releaseElement(FifoGeneric::Chain* link, unsigned int readerId)
{
    if (link != nullptr)
    {
        assert(m_fifo_first != nullptr);

        m_mutex.enter();
        // Reduce expected reads because one is finished. If back to zero remove element from FIFO.
        // This violate the FIFO characteristic but no more pop operations are performed on the element.
        link->expectedReadsBitMask &= ~readerId;
        if (link->expectedReadsBitMask == 0)
        {
            // Search element in FIFO
            if (link == m_fifo_first)
            {
                // Head of FIFO
                m_fifo_first = link->next;
                link->next = m_unused;
                m_unused = link;
                if (m_fifo_last == link)
                {
                    m_fifo_last = nullptr;
                }
            }
            else
            {
                // Not head, find previous to remove link from list.
                Chain* previous = m_fifo_first;
                while ((previous != nullptr) && (previous->next != link))
                {
                    previous = previous->next;
                }
                assert(previous != nullptr);
                // Remove from list if a previous element is found
                if (previous != nullptr)
                {
                    previous->next = link->next;
                    // Correct last pointer it the element to release is the last one.
                    if (link == m_fifo_last)
                    {
                        m_fifo_last = previous;
                    }
                    link->next = m_unused;
                    m_unused = link;
                }
            }
        }
        m_mutex.leave();
    }
}

// -------------------

bool
FifoGeneric::push(const void* data)
{
    bool result = false;
    Chain* element = nullptr;
    // Are there allocated elements?
    if (m_allocated != nullptr)
    {
        m_mutex.enter(); // Should synchronize against other access to the object
        // Is data item head element of allocated list?
        if (m_allocated->data == data)
        {
            // Remove head element of allocated list
            element = m_allocated;
            m_allocated = element->next;
        }
        else
        {
            // Search element in allocated list and remove it from allocated list
            Chain* previous = findPrevious(data);
            if (previous != nullptr)
            {
                element = previous->next;
                previous->next = element->next;
            }
        }
        m_mutex.leave();
    }
    // Insert element in FIFO
    if (element != nullptr)
    {
        element->expectedReadsBitMask = m_readerBitMask;
        m_mutex.enter(); // Should synchronize against other access to the object
        // Check if FIFO is not empty
        if (!isEmpty())
        {
            // FIFO not empty, build chain
            m_fifo_last->next = element;
        }
        else
        {
            // Empty FIFO, set also the first element beside last element
            m_fifo_first = element;
        }
        // element becomes always last element
        m_fifo_last = element;
        element->next = nullptr;
        // Remember on last pushed object
        m_lastPushed = data;
        m_mutex.leave();
        result = true;
    }
    return result;
}

// -------------------

void*
FifoGeneric::pop(void)
{
    void* result = nullptr;
    m_mutex.enter();
    // To pop a element the FIFO is not empty and has no associated reader.
    // If at least one reader is associated use a reader to read.
    if (!isEmpty() && (m_readers == nullptr))
    {
        // Consume first element in FIFO
        Chain* element = m_fifo_first;
        m_fifo_first = element->next;
        // If last element is consumed in FIFO also no last element exist
        if (m_fifo_first == nullptr)
        {
            m_fifo_last = nullptr;
        }
        // Put element in the list of allocated elements
        element->next = m_allocated;
        m_allocated = element;
        result = element->data;
    }
    m_mutex.leave();
    return result;
}

// -------------------

const void*
FifoGeneric::getlastPushed(void) const
{
    return m_lastPushed;
}

// -------------------

bool
FifoGeneric::isEmpty(void) const
{
    return (m_fifo_first == nullptr);
}

// -------------------

FifoGeneric::Chain*
FifoGeneric::findPrevious(const void* data) const
{
    Chain* result = nullptr;
    // If head element of allocated list contain the data there is no previous element.
    if (m_allocated->data != data)
    {
        // Iterate over the list until the list ends or the element is found.
        for (Chain* previous = m_allocated; (previous->next != nullptr) && (result == nullptr);
             previous = previous->next)
        {
            if (previous->next->data == data)
            {
                result = previous;
            }
        }
    }
    return result;
}

// -------------------
// TODO: this method should have a return value if no new reader can be assigned -> not checked yet
void
FifoGeneric::associateReader(FifoGenericReader& reader)
{
    m_readerMutex.enter();
    // Check if reader is in the list of the readers. In this case do nothing because it shall unique.
    for (FifoGenericReader* check = m_readers; check != nullptr; check = check->nextReader)
    {
        if (check == &reader)
        {
            m_readerMutex.leave();
            return;
        }
    }

    reader.nextReader = m_readers;

    unsigned int readerId = 1;
    // Without readers the number of readers is 1, adding the first reader nothing change in the number of readers.
    if (m_readers != nullptr)
    {

        unsigned int tmpBitField = m_readerBitMask;

        // find first 0 bit -> first free reader id
        while ((tmpBitField >>= 1) & 1)
        {
            readerId <<= 1;
        }

        // new reader is one bit higher
        readerId <<= 1;

        // set new reader
        m_readerBitMask |= readerId;
    }

    m_readers = &reader;
    reader.fifo = this;
    reader.readerId = readerId;
    m_readerMutex.leave();

    // Loop over all elements in the FIFO and correct number of open reads in the element
    m_mutex.enter();
    for (Chain* element = m_fifo_first; element != nullptr; element = element->next)
    {
        element->expectedReadsBitMask |= readerId;
    }
    m_mutex.leave();
}

// -------------------

void
FifoGeneric::releaseReader(FifoGenericReader& reader)
{
    // Cleanup allocated elements
    reader.releaseAll();
    reader.linkMutex.enter();
    // Bring first element of the reader in synch to FIFO...
    if (reader.fifo_first == nullptr)
    {
        reader.fifo_first = m_fifo_first;
    }
    // ... to decrease the number of expected reads in all unread elements
    while (reader.fifo_first != nullptr)
    {
        Chain* next = reader.fifo_first->next;
        releaseElement(reader.fifo_first, reader.readerId);
        reader.fifo_first = next;
    }
    // FIFO is now empty, so set last also to zero
    m_fifo_last = nullptr;
    reader.fifo = nullptr;
    reader.linkMutex.leave();
    // Remove reader from the list of readers
    m_readerMutex.enter();
    if (&reader == m_readers)
    {
        // Reader is head of list
        m_readers = m_readers->nextReader;
        // When the reader was the last reader, the normal FIFO has exactly one reader.
        if (m_readers != nullptr)
        {
            // delete the bit
            m_readerBitMask &= ~reader.readerId;
        }
        else
        {
            m_readerBitMask = 0x01;
        }
    }
    else
    {
        // Reader is not head of list, find it and remove from list when found.
        for (FifoGenericReader* currentReader = m_readers; currentReader != nullptr;
             currentReader = currentReader->nextReader)
        {
            if (currentReader->nextReader == &reader)
            {
                currentReader->nextReader = reader.nextReader;
                m_readerBitMask &= ~reader.readerId;
                // Terminate search by setting next to null pointer and currentReader to removed one
                reader.nextReader = nullptr;
                currentReader = &reader;
            }
        }
    }
    m_readerMutex.leave();
}

// -------------------

void
FifoGeneric::synchronize(const Tasking::Task* task, const unsigned int volume)
{
    // If volume is zero no synchronization is necessary
    if (volume > 0)
    {
        // Find in the list of reader of the calling task and synchronize it.
        FifoGenericReader* reader = m_readers;
        m_mutex.enter(); // add due to race condition during terminate in this part of the code
        while (reader != nullptr)
        {
            if (reader->readerTask == task)
            {
                // Determine last element in FIFO by the volume. Two cases, reader was empty or reader wasn't empty.
                Chain* newItem = m_fifo_first;
                assert(newItem != nullptr);
                if (reader->fifo_first == nullptr)
                {
                    reader->fifo_first = m_fifo_first;

                    // Check if the bit in the reader bit mask was already cleared -> element was already consumed by
                    // the reader follow the chain to the first not set bit
                    while (/*(reader->fifo_first != NULL) && */ (
                            (reader->fifo_first->expectedReadsBitMask & reader->readerId) == 0))
                    {
                        reader->fifo_first = reader->fifo_first->next;
                        assert(reader->fifo_first != nullptr);
                    }
                    newItem = reader->fifo_first;
                }
                else
                {
                    // Reader wasn't empty, so it has also a valid last item and the next is the first new item
                    newItem = reader->fifo_last->next;
                    assert(newItem != nullptr);
                }
                // Find new last item in queue (Most of the time it is the last one)
                for (unsigned int i = 0; (i < volume - 1); i++)
                {
                    // Should always a valid pointer else the volume is wrong.
                    assert(newItem->next != nullptr);
                    newItem = newItem->next;
                }
                reader->fifo_last = newItem;
                // Reader is unique, terminate
                break;
            }
            else
            {
                reader = reader->nextReader;
            }
        }
        m_mutex.leave();
    }
}
