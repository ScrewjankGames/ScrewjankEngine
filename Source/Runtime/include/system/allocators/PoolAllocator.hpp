#pragma once
// STD Headers

// Library Headers

// Engine Headers
#include <core/Assert.hpp>
#include <core/Log.hpp>
#include <system/Memory.hpp>
#include <system/Allocator.hpp>
#include <system/Memory.hpp>

namespace sj {

    /**
     * Allocates memory in fixed size chunks
     * @tparam kBlockSize The size of each chunk in the pool
     */
    template <size_t kBlockSize>
    class PoolAllocator : public Allocator
    {
      public:
        /**
         * Constructor
         * @param block_count The number of blocks of size kBlockSize
         */
        PoolAllocator(size_t block_count, void* memory);

        /**
         * Destructor
         */
        ~PoolAllocator();

        /**
         * Allocates size bites from the heap
         * @param size The number of bytes to allocate
         */
        [[nodiscard]] virtual void* Allocate(const size_t size, 
                                             const size_t alignment = alignof(std::max_align_t)) override;

        /**
         * Marks memory as free
         * @param memory Pointer to the memory to free
         */
        virtual void Free(void* memory = nullptr) override;

      private:
        /** Node structure for the for the free block linked list */
        struct FreeBlock
        {
            FreeBlock* Next;
        };

        /** The beginning of this allocator's data buffer */
        void* m_BufferStart;

        /** Pointer head of singly linked list of free blocks */
        FreeBlock* m_FreeListHead;

        /** Number of blocks managed by this allocator */
        size_t m_NumBlocks;

        /** Structure used to track and report the state of this allocator */
        AllocatorStatus m_AllocatorStats;
    };

} // namespace sj

// Include Inlines
#include <system/allocators/PoolAllocator.inl>