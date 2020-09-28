#pragma once
// STD Headers
#include <cstddef>

// Library Headers

// Engine Headers
#include "system/Allocator.hpp"
#include "core/MemorySystem.hpp"

namespace sj {

    class StackAllocator : public Allocator
    {
      public:
        StackAllocator(size_t buffer_size,
                       Allocator* backing_allocator = MemorySystem::GetDefaultAllocator());

        ~StackAllocator();

        [[nodiscard]] void* Allocate(const size_t size, const size_t alignment) override;

        void Free(void* memory = nullptr) override;

        template <typename T>
        [[nodiscard]] void* PushType();

        [[nodiscard]] void* Push(size_t size, size_t alignment = 1);

        void Pop();

      private:
        /** Data structure used to manage allocations the stack */
        struct StackAllocatorHeader
        {
            /** Pointer to the previous header block for popping */
            StackAllocatorHeader* PreviousHeader;

            /** Stores how many bytes were used to pad this header in the stack */
            uintptr_t HeaderOffset;
        };

        /** Allocator from which this allocator requests memory */
        Allocator* m_BackingAllocator;

        /** The size in bytes of the allocator's buffer */
        size_t m_Capacity;

        /** Buffer the allocator manages */
        void* m_BufferStart;

        /** Pointer to the latest allocation's header block */
        StackAllocatorHeader* m_CurrentHeader;

        /** Pointer to the first free byte in the stack */
        void* m_Offset;

        /** Structure used to track and report the state of this allocator */
        AllocatorStatus m_AllocatorStats;
    };

    template <typename T>
    inline void* StackAllocator::PushType()
    {
        return Allocate(sizeof(T), alignof(T));
    }
} // namespace sj
