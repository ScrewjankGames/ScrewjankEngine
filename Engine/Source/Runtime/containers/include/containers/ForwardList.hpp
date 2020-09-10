#pragma once
// STD Headers
#include <initializer_list>

// Library Headers

// Screwjank Headers
#include "system/Memory.hpp"
#include "core/MemorySystem.hpp"

namespace Screwjank {

    template <class T>
    class ForwardList
    {
      public:
        // Type definitions
        using value_type = T;
        using reference = value_type&;
        using const_reference = const reference;
        using pointer = value_type*;

        using difference_type = ptrdiff_t;
        using size_type = size_t;

        ForwardList(Allocator* allocator = MemorySystem::GetDefaultAllocator());

        ~ForwardList();

        T& Front();

      private:
        template <class T>
        struct ForwardListNode
        {
            T Data;
            ForwardListNode* Next;
        };

        /** Pointer to the head of the list */
        UniquePtr<ForwardListNode<T>> m_Head;

        /** Allocator used to service allocations for container structures */
        Allocator* m_Allocator;
    };

    template <class T>
    inline ForwardList<T>::ForwardList(Allocator* allocator) : m_Allocator(allocator)
    {
    }

    template <class T>
    inline ForwardList<T>::~ForwardList()
    {
    }

} // namespace Screwjank
