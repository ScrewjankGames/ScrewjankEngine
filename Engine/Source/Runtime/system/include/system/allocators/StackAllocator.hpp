#pragma once
// STD Headers
#include <cstddef>

// Library Headers

// Engine Headers
#include "system/Allocator.hpp"
#include "core/MemorySystem.hpp"

namespace Screwjank {

    struct StackAllocatorHeader
    {
        std::byte* PreviousHeader;
    };

    class StackAllocator : public Allocator
    {
      public:
        StackAllocator(size_t buffer_size,
                       Allocator* backing_allocator = MemorySystem::GetDefaultAllocator(),
                       const char* debug_name = "");

        ~StackAllocator();

        void* Allocate(const size_t size, const size_t alignment) override;

        void Free(void* memory = nullptr) override;

        template <typename T>
        void* Push();

        void Pop();

        void Reset();

      private:
        /** Allocator from which this allocator requests memory */
        Allocator* m_BackingAllocator;

        /** Buffer the allocator manages */
        void* m_Buffer;

        /** The size in bytes this allocator manages */
        size_t m_Capacity;

        /** Pointer to the start of the current stack frame */
        StackAllocatorHeader* m_CurrFrameHeader;
    };

    template <typename T>
    inline void* StackAllocator::Push()
    {
        return Allocate(sizeof(T), alignof(T));
    }
} // namespace Screwjank
