#include <ScrewjankEngine/system/allocators/PoolAllocator.hpp>

namespace sj
{
    template <class T>
    class ObjectPoolAllocator : public PoolAllocator<sizeof(T)>
    {
        // Inherit constructor
        using PoolAllocator<sizeof(T)>::PoolAllocator;
    };

    template <size_t kBlockSize>
    inline PoolAllocator<kBlockSize>::PoolAllocator(size_t num_blocks, void* memory)
        : m_BufferStart(memory), m_NumBlocks(num_blocks)
    {
        // Ensure block size is large enough to store an allocation header
        static_assert(kBlockSize > sizeof(FreeBlock),
                      "Block size is not large enough to maintain free list");

        SJ_ASSERT(num_blocks > 0, "Cannot create a pool allocator with zero blocks");

        // Create first free block at start of buffer
        m_FreeListHead = new (m_BufferStart) FreeBlock {nullptr};
        FreeBlock* curr_block = m_FreeListHead;
        uintptr_t curr_block_address = (uintptr_t)m_BufferStart;

        // Build free list in the buffer
        for (size_t i = 1; i < num_blocks; i++)
        {
            // Calculate the block address of the next block
            curr_block_address += kBlockSize;

            // Create new free-list block
            curr_block->Next = new ((void*)curr_block_address) FreeBlock();

            // Move the curr block forward
            curr_block = curr_block->Next;
        }
    }

    template <size_t kBlockSize>
    inline PoolAllocator<kBlockSize>::~PoolAllocator()
    {
        SJ_ASSERT(m_AllocatorStats.ActiveAllocationCount == 0, "Memory leak detected");
    }

    template <size_t kBlockSize>
    inline void* PoolAllocator<kBlockSize>::Allocate(const size_t size, const size_t alignment)
    {
        SJ_ASSERT(size <= kBlockSize,
                  "Pool allocator cannot satisfy allocation of size > block size");

        if (m_FreeListHead == nullptr)
        {
            SJ_ENGINE_LOG_ERROR("Pool allocator has run out of blocks");
            return nullptr;
        }

        // Take the first available free block
        auto free_block = m_FreeListHead;
        SJ_ASSERT(IsMemoryAligned(free_block, alignment),
                  "PoolAllocator does not support over-aligned types");

        // Remove the free block from the free list
        m_FreeListHead = m_FreeListHead->Next;

        m_AllocatorStats.TotalAllocationCount++;
        m_AllocatorStats.TotalBytesAllocated += kBlockSize;
        m_AllocatorStats.ActiveAllocationCount++;
        m_AllocatorStats.ActiveBytesAllocated += kBlockSize;

        return (void*)(free_block);
    }

    template <size_t kBlockSize>
    inline void PoolAllocator<kBlockSize>::Free(void* memory)
    {
        // Ensure memory is in the region managed by this allocator
        SJ_ASSERT((uintptr_t)memory >= (uintptr_t)m_BufferStart,
                  "Memory is not managed by this allocator");
        SJ_ASSERT((uintptr_t)memory <= (uintptr_t)m_BufferStart + (kBlockSize * m_NumBlocks),
                  "Memory is not managed by this allocator");

        // Place a free-list node into the block
        auto new_block = new (memory) FreeBlock {m_FreeListHead};

        // Push block onto head of free list
        m_FreeListHead = new_block;

        m_AllocatorStats.ActiveAllocationCount--;
        m_AllocatorStats.ActiveBytesAllocated -= kBlockSize;
    }
}