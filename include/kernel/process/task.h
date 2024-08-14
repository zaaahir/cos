#ifndef TASK_H
#define TASK_H

#include "fs/vfs.h"
#include "common/doublyLinkedList.h"
#include "event/dispatcher.h"
#include "common/utilities.h"
#include "memory/MemoryManager.h"
#include "memory/AddressSpace.h"

namespace Task
{
    static constexpr uint64_t SIZE_OF_IRETQ_STACK = 20*8;
    using TaskID = uint64_t;

    // A SharableTaskResource manages a resource between tasks using reference counting.
    template <class T>
    class SharableTaskResource
    {
    public:
        SharableTaskResource(T resource)
        {
            // Allocate a new control block with the resource.
            m_controlBlock = new SharableTaskResourceControlBlock(resource);
            m_controlBlock->m_resource = resource;
            m_controlBlock->m_refCount++;
        }

        SharableTaskResource(const SharableTaskResource& sharedResource)
        {
            // We are constructing with another shared resource, so copy the control block and increment refcount.
            m_controlBlock = sharedResource.m_controlBlock;
            m_controlBlock->m_refCount++;
        }

        SharableTaskResource& operator=(SharableTaskResource sharedResource)
        {
            // We are constructing with another shared resource, so copy the control block and increment refcount.
            auto tempControlBlock = sharedResource.m_controlBlock;
            sharedResource.m_controlBlock = this->m_controlBlock;
            this->m_controlBlock = tempControlBlock;
            return *this;
        }

        SharableTaskResource(SharableTaskResource&& sharedResource)
        {
            // Using move semantics, we steal the control block from the old shared resource.
            m_controlBlock = sharedResource.m_controlBlock;
            sharedResource.m_controlBlock = nullptr;
        }

        SharableTaskResource& operator=(SharableTaskResource&& sharedResource)
        {
            // Using move semantics, we steal the control block from the old shared resource.
            m_controlBlock = sharedResource.m_controlBlock;
            sharedResource.m_controlBlock = nullptr;
            return *this;
        }

        T& operator*()
        {
            // Return the underlying resource
            return m_controlBlock->m_resource;
        }

        ~SharableTaskResource()
        {
            if (m_controlBlock)
            {
                m_controlBlock->m_refCount--;
                if (m_controlBlock->m_refCount == 0)
                {
                    delete m_controlBlock;
                }
            }
        }
    }
  }
}

#endif
