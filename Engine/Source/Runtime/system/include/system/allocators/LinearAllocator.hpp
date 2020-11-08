#pragma once
// STD Headers

// Library Headers

// Engine Headers
#include "system/Memory.hpp"
#include "system/Allocator.hpp"

namespace sj {

    class LinearAllocator : public Allocator
    {
      public:
        /**
         * Constructor
         * @param buffer_size The size of this allocator's buffer in bytes
         * @param backing_allocator The allocator this allocator get's it's memory from
         */
        LinearAllocator(size_t buffer_size,
                        Allocator* backing_allocator = MemorySystem::GetDefaultAllocator());

        /**
         * Destructor
         */
        ~LinearAllocator();

        /**
         * Allocates size bites from the heap
         * @param size The number of bytes to allocate
         * @param alignment The alignment requirement for this allocation
         */
        [[nodiscard]] void* Allocate(const size_t size,
                                     const size_t alignment = alignof(std::max_align_t)) override;

        /**
         * Attempts to free a memory address
         * @param memory Pointer to the memory to free
         * @note Linear Allocators don't support the free operation, and will assert failure if
         * called
         */
        void Free(void* memory = nullptr) override;

        /**
         * Marks all allocations for this allocator as invalid and frees the buffer for more
         * allocations
         */
        void Reset();

      private:
        /** Allocator used to aquire and release this allocators buffer */
        Allocator* m_BackingAllocator;

        /** Pointer to the start of the allocator's managed memory */
        void* m_BufferStart;

        /** Pointer to the first free byte in the linear allocator */
        void* m_CurrFrameStart;

        /** Structure used to track and report the state of this allocator */
        AllocatorStatus m_AllocatorStats;
    };

} // namespace sj
