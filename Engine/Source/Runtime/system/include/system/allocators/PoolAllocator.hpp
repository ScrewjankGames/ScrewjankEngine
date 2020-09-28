#pragma once
// STD Headers

// Library Headers

// Engine Headers
#include "core/Assert.hpp"
#include "core/MemorySystem.hpp"
#include "system/Allocator.hpp"
#include "system/Memory.hpp"

namespace sj {

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
         * Destructor
         */
        ~PoolAllocator();

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

        /** Structure used to track and report the state of this allocator */
        AllocatorStatus m_AllocatorStats;
    };

    template <class T>
    class ObjectPoolAllocator : public PoolAllocator<sizeof(T)>
    {
        // Inherit constructor
        using PoolAllocator<sizeof(T)>::PoolAllocator;
    };

    template <size_t t_BlockSize>
    inline PoolAllocator<t_BlockSize>::PoolAllocator(size_t num_blocks,
                                                     Allocator* backing_allocator)
        : m_BackingAllocator(backing_allocator), m_NumBlocks(num_blocks)
    {
        // Ensure block size is large enough to store an allocation header
        static_assert(t_BlockSize > sizeof(FreeBlock),
                      "Block size is not large enough to maintain free list");

        SJ_ASSERT(num_blocks > 0, "Cannot create a pool allocator with zero blocks");

        m_BufferStart =
            m_BackingAllocator->Allocate(num_blocks * t_BlockSize, alignof(std::max_align_t));

        // Create first free block at start of buffer
        uintptr_t block_address = (uintptr_t)m_BufferStart;

        SJ_ASSERT(IsMemoryAligned(m_BufferStart, alignof(FreeBlock)),
                  "Memory misalignment detected");

        m_FreeList = new ((void*)block_address) FreeBlock {nullptr};
        FreeBlock* curr_block = m_FreeList;

        // Build free list in the buffer
        for (size_t i = 1; i < num_blocks; i++) {
            // Calculate the block address of the next block
            block_address += t_BlockSize;

            // Create new free-list block
            curr_block->Next = new ((void*)block_address) FreeBlock();

            // Move the curr block forward
            curr_block = curr_block->Next;
        }
    }

    template <size_t t_BlockSize>
    inline PoolAllocator<t_BlockSize>::~PoolAllocator()
    {
        SJ_ASSERT(m_AllocatorStats.ActiveAllocationCount == 0, "Memory leak detected");

        m_BackingAllocator->Free(m_BufferStart);
    }

    template <size_t t_BlockSize>
    inline void* PoolAllocator<t_BlockSize>::Allocate(const size_t size, const size_t alignment)
    {
        SJ_ASSERT(size <= t_BlockSize,
                  "Pool allocator cannot satisfy allocation of size > block size");

        if (m_FreeList == nullptr) {
            SJ_ENGINE_LOG_ERROR("Pool allocator has run out of blocks");
            return nullptr;
        }

        // Take the first available free block
        auto free_block = m_FreeList;
        auto adjustment = GetAlignmentAdjustment(alignment, free_block);

        if (size + adjustment > t_BlockSize) {
            SJ_ENGINE_LOG_ERROR(
                "Pool allocator cannot service allocation of size {} with alignof {}",
                size,
                alignment);

            return nullptr;
        }

        // Remove the free block from the free list
        m_FreeList = m_FreeList->Next;

        m_AllocatorStats.TotalAllocationCount++;
        m_AllocatorStats.TotalBytesAllocated += t_BlockSize;
        m_AllocatorStats.ActiveAllocationCount++;
        m_AllocatorStats.ActiveBytesAllocated += t_BlockSize;

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

        // Find out how far memory address is from the block start
        auto offset = GetAlignmentOffset(t_BlockSize, memory);

        // Get the starting address of the block
        void* block_address = (void*)((uintptr_t)memory - offset);

        SJ_ASSERT(GetAlignmentOffset(t_BlockSize, block_address) == 0,
                  "Block start not found correctly");

        auto new_block = new (block_address) FreeBlock {m_FreeList};

        // Push block onto head of free list
        m_FreeList = new_block;

        m_AllocatorStats.ActiveAllocationCount--;
        m_AllocatorStats.ActiveBytesAllocated -= t_BlockSize;
    }

} // namespace sj
