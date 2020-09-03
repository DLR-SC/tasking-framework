/*
 * fifoGeneric.h
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

#ifndef TASKING_FIFO_GENERIC_H_
#define TASKING_FIFO_GENERIC_H_

#include <mutex.h>
#include <task.h>

namespace Tasking {

  class FifoGenericReader;

  /**
   * Generic FIFO stack which operates on void pointers. Application developer should use
   * the template class Fifo instead of this implementation.
   *
   * @see Fifo
   */
  class FifoGeneric {
      friend class FifoGenericReader;
    public:

      /// Structure to manage memory of the FIFO
      struct Chain {
	  /// Pointer to the previous memory element
	  Chain *next;
	  /// One data item
	  void* data;
	  /// Number of remaining read calls from FIFO reader. Release in a reader is only possible when the number is zero.
	  unsigned int expectedReadsBitMask;
      };

      /**
       * Initialize the generic FIFO stack.
       *
       * @param chain Pointer to the buffer area used for the management of the FIFO stack
       * @param dataBuffer Pointer to the area for the data items provided by the FIFO stack
       * @param size Size of one data item in the buffer area
       * @param items Number of data items in the buffer area
       */
      FifoGeneric(struct FifoGeneric::Chain* chain, void* dataBuffer, const size_t size, const unsigned int items);

      /**
       * Reserve a data item provided by the FIFO stack.
       *
       * @result Pointer to a data item. When the FIFO stack has no more free memory it delivers a NULL pointer.
       */
      void* allocate(void);

      /**
       * Release a pointer for further use without pushing.
       *
       * @param data Pointer to a data item provided by the FIFO stack. The data shall be provided by the FIFO stack
       * with a call to allocate or pop, if not no operations is performed.
       * @param readerId bit mask of the reader that releases the element, if no reader is used, it is set to 1
       *
       * @see allocate
       * @see pop
       */
      void release(const void* data, unsigned int readerId = 1);

      /**
       * Push a data element into the FIFO stack.
       *
       * @param data Pointer to the data to push. The data shall provided by the FIFO stack with a call to allocate
       * or pop, else no operation is performed.
       *
       * @result True if operation was successful
       */
      bool push(const void* data);

      /**
       * Request the oldest data in the FIFO stack and consume it. The data item is allocated by the caller and it shall
       * call release to provide the memory of the element for further use. If release is not called, the software has a
       * memory leak and allocation is not possible if all data items are allocated.
       *
       * @result Pointer to the oldest data in the FIFO stack.
       *
       * @see release
       */
      void* pop(void);

      /**
       * Request the last pushed element. Attention
       */
      const void* getlastPushed(void) const;

      /**
       * Request if a push operation will deliver data from the FIFO stack.
       *
       * @result True if data is available, else false.
       */
      bool isEmpty(void) const;

      /**
       * Associate a reader to the FIFO. The reader must be unique to the FIFO, else nothing is done. When a reader
       * is associated during run time of the system elements currently in the FIFO becomes also visible to the reader.
       *
       * @param reader Reference to the reader which should associated with the FIFO.
       */
      void associateReader(FifoGenericReader& reader);

      /**
       * Release an associated reader from the FIFO. The associated reader shouldn't release popped data, else the
       * FIFO property in other readers can lost. The associated task to the reader shouldn't run during the reader
       * is released. If not the FIFO property to other readers can lost.
       *
       * @param reader Pointer to the FIFO reader to release.
       */
      void releaseReader(FifoGenericReader& reader);

      /**
       * A task start to work on the message data. The method will adjust the information on the FIFO state
       * for associated FIFO reader.
       *
       * @param task Pointer to the task which will work on the data.
       * @param volume Number of activations of the associated task input.
       */
      void synchronize(const Tasking::Task* task, const unsigned int volume);

    protected:

      /**
       * Release an element in the FIFO. This method is used by FIFO readers to release a popped element.
       * When the number of read accesses is down to zero the element is removed from the FIFO.
       *
       * @param link Pointer to the element to release.
       * @param readerId bit mask of the reader that releases the element, if no reader is used, it is set to 1
       */
      void releaseElement(Chain* link, unsigned int readerId = 1);

    private:

      /**
       * Find in the allocated list the chaining element which is previous to the associated
       * to the data element.
       *
       * @param data Pointer to the data item which shall associated to the following chain element.
       *
       * @result Pointer to the previous chain element to the one associated with the data item.
       */
      Chain* findPrevious(const void* data) const;

      /// Pointer to a chain of unused data items
      Chain* m_unused;
      /// Pointer to a chain of allocated data items
      Chain* m_allocated;
      /// Pointer to the first item in the FIFO stack. This item is consumed by pop.
      Chain* m_fifo_first;
      /// Pointer to the last item in the FIFO stack. This item is the last pushed.
      Chain* m_fifo_last;
      /// Pointer to the last pushed data. This is also delivered when the queue is empty.
      const void* m_lastPushed;
      /// Mutex to synchronize data access to the object.
      Tasking::Mutex m_mutex;
      /// Pointer to the list of readers associated to this FIFO
      FifoGenericReader* m_readers;
      /// Bitmask of readers associated with this FIFO. To simplify logic the first bit is always set to 1 without associated readers.
      unsigned char m_readerBitMask;
      /// Mutex to synchronize access to reader list. The mutex is static because access to the list of readers are seldom.
      Tasking::Mutex m_readerMutex;

  };
// FifoGeneric

  /**
   * Class to read from a FIFO with several readers. Each reader assigned to the FIFO see the FIFO characteristic.
   */
  class FifoGenericReader {

      friend class FifoGeneric;

    public:

      /**
       * A generic FIFO read is always associated with a task. The first constructor call will set up static elements.
       * @param task Pointer to the task the reader is connected to.
       * @param fifo Associated fifo of the reader.
       */
      explicit FifoGenericReader(Tasking::Task* task);

      /**
       * Cleanup static elements in the FIFO reader.
       */
      ~FifoGenericReader(void);

      /**
       * Request if a push operation will deliver data from the FIFO stack.
       *
       * @result True if data is available, else false.
       */
      bool isEmpty(void) const;

      /**
       * Release all by the reader popped data.
       */
      void releaseAll(void);

    protected:

      /**
       * Release a pointer for this reader. The element can be used again when all assigned reader release the pointer.
       *
       * @param data Pointer to a data item provided by the FIFO reader. If the data is not provided by the
       * FIFO reader nothing happens.
       */
      void release(void* data);

      /**
       * Request the oldest data in the FIFO stack and consume it. The data item is allocated by the caller and it shall
       * call release to provide the memory of the element for further use. If release is not called, the software has a
       * memory leak and allocation is not possible if all data items are allocated.
       *
       * @result Pointer to the oldest data in the FIFO stack.
       *
       * @see release
       */
      void* pop(void);

      /// Structure to manage by the reader popped elements from the FIFO
      struct Chain {
	  /// Pointer to the allocated element in the FIFO
	  FifoGeneric::Chain* fifoElement;
	  /// Pointer to the next link in the chain
	  Chain* next;
      };

      /// Pointer to the next reader or null pointer if last reader in List.
      FifoGenericReader* nextReader;

      /// Reader number, starts with 0 and maximum (numberOfBits(unsigned int))
      unsigned int readerId;

      /// The pointer to the task which use this FIFO reader.
      Task* readerTask;

      /// Pointer to the associated FIFO. This pointer is set by the FIFO when the reader is associated to them.
      FifoGeneric* fifo;

      /// Pointer to the first item in the FIFO stack. This item is consumed by pop.
      FifoGeneric::Chain* fifo_first;

      /// Pointer to the last item in the FIFO stack. This item is the last pushed.
      FifoGeneric::Chain* fifo_last;

      /// By the reader allocated elements in the FIFO
      struct Chain* allocated_elements;

      /// Mutex to synchronize the access to unusedLinks
      Tasking::Mutex linkMutex;

      /// Unused links in the pool of chain links
      struct Chain* unusedLinks;

      static const int maximumFifoReaderLinks = 2000; // TODO this shouldn't be a constant and needs to be the same as fifo

      /// Pool of chain links for the management of links.
      struct Chain links[maximumFifoReaderLinks];

  };
// FifoGenericReader

}// namespace Tasking

#endif /* TASKING_FIFO_GENERIC_H_ */
