#pragma once
// STD Headers
#include <utility>

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

        /**
         * Marks memory as free
         * @param memory Pointer to the memory to free
         */
        void Free(void* memory = nullptr) override;

      private:
        /** Linked list node structure inserted in-place into the allocator's buffer */
        struct FreeBlock
        {
            size_t Size;
            FreeBlock* Previous;
            FreeBlock* Next;
        };

        struct AllocationHeader
        {
            /** The padding placed before this header in the free block during allocation */
            size_t Padding;

            /** Size of the allocated block */
            size_t Size;
        };

        [[nodiscard]] std::pair<FreeBlock*, size_t> FindFreeBlock(const size_t size,
                                                                  const size_t alignment);

        void RegisterNewFreeBlock(FreeBlock* block);

        /** Allocator used by this allocator to get and free memory */
        Allocator* m_BackingAllocator;

        /** The free list of allocation blocks */
        FreeBlock* m_FreeBlocks;

        /** Pointer to the start of the allocator's memory block */
        void* m_BufferStart;
    };
} // namespace Screwjank
