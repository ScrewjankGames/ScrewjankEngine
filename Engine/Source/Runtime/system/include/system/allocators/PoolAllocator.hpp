#pragma once
// STD Headers

// Library Headers

// Engine Headers
#include "core/Assert.hpp"
#include "core/MemorySystem.hpp"
#include "system/Allocator.hpp"
#include "system/Memory.hpp"

namespace Screwjank {

    /**
     * Allocates memory in fixed size chunks
     */
    template <size_t t_BlockSize>
    class PoolAllocator : public Allocator
    {
      public:
        /**
         * Constructor
         * @param block_count The number of blocks of size t_BlockSize
         */
        PoolAllocator(size_t block_count,
                      Allocator* backing_allocator = MemorySystem::GetDefaultAllocator());

        /**
         * Allocates size bites from the heap
         * @param size The number of bytes to allocate
         */
        [[nodiscard]] virtual void* Allocate(const size_t size,
                                             const size_t alignment = 1) override;

        /**
         * Marks memory as free
         * @param memory Pointer to the memory to free
         */
        virtual void Free(void* memory = nullptr) override;

      private:
        struct FreeBlock
        {
            FreeBlock* Next;
        };

        Allocator* m_BackingAllocator;

        /** The beginning of this allocator's data buffer */
        void* m_BufferStart;

        /** Pointer head of singly linked list of free blocks */
        FreeBlock* m_FreeList;

        /** Number of blocks managed by this allocator */
        size_t m_NumBlocks;
    };

    template <size_t t_BlockSize>
    inline PoolAllocator<t_BlockSize>::PoolAllocator(size_t num_blocks,
                                                     Allocator* backing_allocator)
        : m_BackingAllocator(backing_allocator), m_NumBlocks(num_blocks)
    {
        // Ensure block size is large enough to store an allocation header
        static_assert(t_BlockSize > sizeof(FreeBlock),
                      "Block size is not large enough to maintain free list");

        SJ_ASSERT(num_blocks > 0, "");

        m_BufferStart =
            m_BackingAllocator->Allocate(num_blocks * t_BlockSize, alignof(std::max_align_t));

        uintptr_t block_address = (uintptr_t)m_BufferStart;
        m_FreeList = new ((void*)block_address) FreeBlock {nullptr};
        FreeBlock* curr_block = m_FreeList;

        // Build free list
        for (size_t i = 1; i < num_blocks; i++) {
            // Calculate the block address of the next block
            block_address += (i * t_BlockSize);

            // Create new free-list block
            curr_block->Next = new ((void*)block_address) FreeBlock();

            // Move the curr block forward
            curr_block = curr_block->Next;
        }
    }

    template <size_t t_BlockSize>
    inline void* PoolAllocator<t_BlockSize>::Allocate(const size_t size, const size_t alignment)
    {
        SJ_ASSERT(size <= t_BlockSize,
                  "Pool allocator cannot satisfy allocation of size > block size");

        if (m_FreeList == nullptr) {
            SJ_ENGINE_LOG_ERROR("Pool allocator {} has run out of blocks", m_DebugName);
            return nullptr;
        }

        // Pull the first block off the free list, and update and remove it from the free list
        void* free_block = m_FreeList;
        m_FreeList = m_FreeList->Next;

        // Get required adjustment to align allocation in block
        auto adjustment = GetAlignmentAdjustment(alignment, free_block);

        if (size + adjustment > t_BlockSize) {
            SJ_ENGINE_LOG_ERROR("Pool allocator {} cannot satisfy alignment requirement",
                                m_DebugName);
            return nullptr;
        }

        return (void*)((uintptr_t)free_block + adjustment);
    }

    template <size_t t_BlockSize>
    inline void PoolAllocator<t_BlockSize>::Free(void* memory)
    {
        // Ensure memory is in the region managed by this allocator
        SJ_ASSERT((uintptr_t)memory >= (uintptr_t)m_BufferStart,
                  "Memory is not managed by this allocator");
        SJ_ASSERT((uintptr_t)memory <= (uintptr_t)m_BufferStart + (t_BlockSize * m_NumBlocks),
                  "Memory is not managed by this allocator");

        // Find out how far memory is from the start of a block
        auto offset = GetAlignmentOffset(t_BlockSize, memory);

        // Create new free block
        auto new_block = new ((void*)((uintptr_t)memory - offset)) FreeBlock {nullptr};

        // Add new free block to head of list
        new_block->Next = m_FreeList;
        m_FreeList = new_block;
    }

} // namespace Screwjank
