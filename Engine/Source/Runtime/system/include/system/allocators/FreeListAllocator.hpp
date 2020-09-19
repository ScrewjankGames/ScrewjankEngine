#pragma once
// STD Headers

// Library Headers

// Screwjank Headers
#include "core/MemorySystem.hpp"
#include "containers/ForwardList.hpp"
#include "system/Allocator.hpp"

namespace Screwjank {

    class FreeListAllocator : public Allocator
    {
      public:
        FreeListAllocator(size_t buffer_size,
                          Allocator* backing_allocator = MemorySystem::GetDefaultAllocator(),
                          const char* debug_name = "");

        ~FreeListAllocator();

        /**
         * Allocates size bites with given alignment in a best-fit manner
         * @param size The number of bytes to allocate
         */
        [[nodiscard]] void* Allocate(const size_t size, const size_t alignment = 1) override;

      private:
        /** Linked list node structure inserted in-place into the allocator's buffer */
        struct FreeBlock
        {
            size_t Size;
            FreeBlock* Next;
        };

        struct AllocationHeader
        {
            size_t Size;
        };

        Allocator* m_BackingAllocator;

        FreeBlock* m_FreeBlocks;
        void* m_BufferStart;
    };
} // namespace Screwjank
