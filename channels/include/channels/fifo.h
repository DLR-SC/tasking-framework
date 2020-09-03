/*
 * fifo.h
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

#ifndef TASKING_FIFO_H_
#define TASKING_FIFO_H_

#include <task.h>
#include <taskChannel.h>
#include <mutex.h>

#include "fifoGeneric.h"

namespace Tasking {

  /**
   * Class to read from a FIFO with several readers. Each reader assigned to the FIFO see the FIFO characteristic.
   */
  template<typename T> class FifoReader: public FifoGenericReader {
    public:

      explicit FifoReader(Tasking::Task* task);

      /**
       * Release a pointer for this reader. The element can used again when all assigned reader release the pointer.
       *
       * @param data Pointer to a data item provided by the FIFO reader. If the data is not provided by the
       * FIFO reader nothing happens.
       */
      void release(T* data);

      /**
       * Request the oldest data in the FIFO stack and consume it. The data item is allocated by the caller and it shall
       * call release to provide the memory of the element for further use. If release is not called, the software has a
       * memory leak and allocation is not possible if all data items are allocated.
       *
       * @result Pointer to the oldest data in the FIFO stack or null pointer if no further data is in the FIFO.
       *
       * @see release
       */
      T* pop(void);

  };

  /**
   * FIFO stack which overload the functionality of a task message. It also provide memory to the caller
   * to avoid copy actions of long data. If more than one task receive this task message, the data is not
   * shared between the tasks.
   *
   * An identifier can be assigned to the FIFO. It can be _either_ a numeric id or a name of up to
   * 4 characters in length. Only use the respective setter/getter.
   *
   * @param T Data type managed by the FIFO stack.
   * @param size Number of items the FIFO stack can hold.
   */
  template<typename T, unsigned int size> class Fifo: public Channel {
    public:

      /**
       * Initialize the FIFO stack as empty FIFO.
       *
       * @param channelId Identifier for this FIFO.
       *                  Only a name or a channelId can be used for FIFO identification.
       *
       * NOTE:
       *    It is the responsibility of the user to ensure uniqueness of the channel id.
       */
      explicit Fifo(ChannelId channelId = 0);

      /**
       * Initialize the FIFO stack as empty FIFO.
       *
       * @param channelName Null-terminated string specifying a name for this FIFO. The name will be
       *                    truncated after 4 characters. Only a name _or_ a channelId can be used for
       *                    FIFO identification.
       */
      explicit Fifo(const char* channelName);

      virtual ~Fifo(void) {
      }

      /**
       * Reserve a data item provided by the FIFO stack.
       *
       * @result Pointer to a data item. When the FIFO stack has no more free memory it delivers a NULL pointer.
       */
      T* allocate(void);

      /**
       * Release a pointer for further use without pushing.
       *
       * @param data Pointer to a data item provided by the FIFO stack. If the data is not provided by the
       * FIFO stack nothing happens.
       */
      void release(T* data);

      /**
       * Push a data element into the FIFO stack.
       *
       * @param data Pointer to the data to push. The data shall provided by the FIFO stack with a call to allocate
       * or pop, else no operation is performed.
       *
       * @result True if operation was successful
       */
      bool push(const T* data);

      /**
       * Push data into the FIFO stack.
       *
       * @param data Reference to the data. A copy will push on the FIFO. No allocation of the data is necessary before.
       *
       * @result True if operation was successful
       */
      bool push(const T& data);

      /**
       * Request the oldest data in the FIFO stack and consume it. The data item is allocated by the caller and it shall
       * call release to provide the memory of the element for further use. If release is not called, the software has a
       * memory leak and allocation is not possible if all data items are allocated.
       *
       * @result Pointer to the oldest data in the FIFO stack or null pointer if no further data is in the FIFO. If a
       * FIFO reader is associated to the FIFO the result is always a null pointer.
       *
       * @see release
       * @see associatedReader
       */
      T* pop(void);

      /**
       * Request if a push operation will deliver data from the FIFO stack.
       *
       * @result True if data is available, else false.
       */
      bool isEmpty(void) const;

      /**
       * Associate a reader to the FIFO.
       *
       * @param reader Pointer to the reader which should associated with the FIFO.
       */
      void associateReader(FifoReader<T>& reader);

      /**
       * Release an associated reader from the FIFO.
       *
       * @param reader Pointer to the FIFO reader to release.
       */
      void releaseReader(FifoReader<T>& reader);

    protected:

      using Tasking::Channel::push;

      FifoGeneric genericFifo;

      /**
       * A task start to work on the message data. The method will adjust the information on the FIFO state
       * for associated FIFO reader.
       *
       * @param task Pointer to the task which will work on the data.
       * @param volume Number of activations of the associated task input.
       */
      void synchronizeStart(const Tasking::Task* task, const unsigned int volume) override;

      /**
       * Request the last pushed element. Attention
       */
      const T* getlastPushed(void) const;

    private:

      /// Memory area to hold data of the specified size
      T m_data[size];

      /// Memory area for the data management in the generic FIFO stack implementation
      FifoGeneric::Chain m_management[size];

      /// Mutex to prevent race condition in push()
      Tasking::Mutex m_fifo_mutex;

  };

//----- inlines -----

  // cppcheck-suppress uninitMemberVar -- m_management is initialized by genericFifo()
  template<typename T, unsigned int size> Fifo<T, size>::Fifo(ChannelId channelId) :
      Channel(channelId),
      genericFifo(m_management, m_data, sizeof(T), size) {
  }

  template<typename T, unsigned int size> Fifo<T, size>::Fifo(const char* channelName) :
      Channel(channelName),
      genericFifo(m_management, m_data, sizeof(T), size) {
  }

  template<typename T, unsigned int size> T* Fifo<T, size>::allocate(void) {
    m_fifo_mutex.enter();
    T* element = static_cast<T*>(genericFifo.allocate());
    m_fifo_mutex.leave();
    return element;
  }

  template<typename T, unsigned int size> void Fifo<T, size>::release(T* data) {
    m_fifo_mutex.enter();
    genericFifo.release(data);
    m_fifo_mutex.leave();
  }

  template<typename T, unsigned int size> bool Fifo<T, size>::push(const T* data) {
    bool result = false;
    // Only when memory is allocated push the channel.
    m_fifo_mutex.enter();
    if (genericFifo.push(data)) {
      Channel::push();
      result = true;
    }
    m_fifo_mutex.leave();
    return result;
  }

  template<typename T, unsigned int size> bool Fifo<T, size>::push(const T& data) {
    bool result = false;
    T* target = allocate();
    // Only when memory is allocated push the data.
    if (target != NULL) {
      *target = data;
      result = push(target);
    }
    return result;
  }

  template<typename T, unsigned int size> T* Fifo<T, size>::pop(void) {
    return reinterpret_cast<T*>(genericFifo.pop());
  }

  template<typename T, unsigned int size> void Fifo<T, size>::associateReader(FifoReader<T>& reader) {
    genericFifo.associateReader(reader);
  }

  template<typename T, unsigned int size> void Fifo<T, size>::releaseReader(FifoReader<T>& reader) {
    genericFifo.releaseReader(reader);
  }

  template<typename T, unsigned int size> bool Fifo<T, size>::isEmpty(void) const {
    return genericFifo.isEmpty();
  }

  template<typename T, unsigned int size> void Fifo<T, size>::synchronizeStart(const Tasking::Task* task,
									       const unsigned int volume) {
    m_fifo_mutex.enter();
    genericFifo.synchronize(task, volume);
    m_fifo_mutex.leave();
  }

  template<typename T, unsigned int size> const T* Fifo<T, size>::getlastPushed(void) const {
    return static_cast<const T*>(genericFifo.getlastPushed());
  }

  template<typename T> FifoReader<T>::FifoReader(Tasking::Task* task) :
      FifoGenericReader(task) {
  }

  template<typename T> void FifoReader<T>::release(T* data) {
    FifoGenericReader::release(data);
  }

  template<typename T> T* FifoReader<T>::pop(void) {
    return static_cast<T*>(FifoGenericReader::pop());
  }
}  // namespace TASKING

#endif /* FIFO_H_ */
