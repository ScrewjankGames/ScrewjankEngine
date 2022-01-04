#pragma once
// STD Headers

// Library Headers

// Engine Headers
#include <ScrewjankEngine/containers/UnmanagedForwardList.hpp>
#include <ScrewjankEngine/core/Assert.hpp>
#include <ScrewjankEngine/core/Log.hpp>
#include <ScrewjankEngine/system/Memory.hpp>
#include <ScrewjankEngine/system/Allocator.hpp>
#include <ScrewjankEngine/system/Memory.hpp>

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
         * Default constructor 
         */
        PoolAllocator() = default;

        /**
         * Constructor
         * @param block_count The number of blocks of size kBlockSize
         */
        PoolAllocator(size_t buffer_size, void* memory);

        /**
         * Destructor
         */
        ~PoolAllocator();

        void Init(size_t buffer_size, void* memory) override;

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
            FreeBlock* Next = nullptr;

            FreeBlock* GetNext() { return Next; }
            void SetNext(FreeBlock* next) { Next = next; }
        };

        /** The beginning of this allocator's data buffer */
        void* m_BufferStart;

        /** Pointer head of singly linked list of free blocks */
        UnmanagedForwardList<FreeBlock> m_FreeList;

        /** Number of blocks managed by this allocator */
        size_t m_NumBlocks;

        /** Structure used to track and report the state of this allocator */
        AllocatorStatus m_AllocatorStats;
    };

} // namespace sj

// Include Inlines
#include <ScrewjankEngine/system/allocators/PoolAllocator.inl>