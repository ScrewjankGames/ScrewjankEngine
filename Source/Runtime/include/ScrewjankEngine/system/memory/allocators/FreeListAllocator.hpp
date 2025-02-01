#pragma once
// STD Headers
#include <algorithm>
#include <utility>

// Library Headers

// Screwjank Headers
#include <ScrewjankEngine/system/memory/Allocator.hpp>

namespace sj {

    class FreeListAllocator final : public Allocator
    {
      public:
        /**
         * Constructor  
         */
        FreeListAllocator();

        /**
         * Initializing Constructor
         */
        FreeListAllocator(size_t buffer_size, void* memory);

        /**
         * Copy Constructor 
         */
        FreeListAllocator(const FreeListAllocator& other) = delete;

        /**
         * Move Constructor 
         */
        FreeListAllocator(FreeListAllocator&& other) noexcept;

        /**
         * Destructor
         */
        ~FreeListAllocator();

        /*
         * @param buffer_size The size (in bytes) of the memory buffer being managed
         * @param memory The memory this allocator should manage
         */
        void Init(size_t buffer_size, void* memory) override;

        /**
         * Allocates size bites with given alignment in a best-fit manner
         * @param size The number of bytes to allocate
         */
        [[nodiscard]] void* Allocate(const size_t size,
                                     const size_t alignment = alignof(std::max_align_t)) override;

        /**
         * Marks memory as free
         * @param memory Pointer to the memory to free
         */
        void Free(void* memory = nullptr) override;

        /**
         * @return whether the allocator is in a valid state 
         */
        bool IsInitialized() const;

        /**
         * @return whether memory provided is managed by this allocator 
         */
        bool IsMemoryInRange(void* memory) const;

        uintptr_t Begin() const override;
        uintptr_t End() const override;

      private:

        /** Linked list node structure inserted in-place into the allocator's buffer */
        struct FreeBlock
        {
            size_t Size;
            FreeBlock* Previous;
            FreeBlock* Next;

            /** Constructor */
            FreeBlock(size_t block_size = 0, FreeBlock* prev = nullptr, FreeBlock* next = nullptr)
                : Size(block_size), Previous(prev), Next(next)
            {
            }
        };

        /** Book-keeping structure to correctly de-allocate memory */
        struct AllocationHeader
        {
            /** The padding placed before this header in the free block during allocation */
            size_t Padding;

            /** Number of bytes remaining in the current allocation */
            size_t Size;

            /** Constructor */
            AllocationHeader(size_t padding = 0, size_t size = 0) : Padding(padding), Size(size)
            {
            }
        };

        /** Constant value to determine the minimum size of a block */
        static constexpr size_t kMinBlockSize =
            std::max(sizeof(FreeBlock), sizeof(AllocationHeader) + 1);

        /** Constant value to determine the strictest alignment that must be satisfied to split off
         * a new free block */
        static constexpr size_t kMaxMetadataAlignment =
            std::max(alignof(FreeBlock), alignof(AllocationHeader));

        /**
         * Searches the free list for the first block that can satisfy the size and alignment
         * requirements provided, in addition to the allocation header.
         * @param size The size the user wishes to allocate
         * @param the alignment restriction for the memory the user is requesting
         * @return The most suitable free block, along with the amount of padding that needs to be
         * placed before the allocation header to satisfy the allocation in that block
         */
        [[nodiscard]] std::pair<FreeBlock*, size_t> FindFreeBlock(const size_t size,
                                                                  const size_t alignment);

        /**
         * Adds a free block to the free list
         */
        void AddFreeBlock(FreeBlock* new_block);

        /**
         * Removes a free block from the free list
         */
        void RemoveFreeBlock(FreeBlock* block);

        /**
         * Given a block in the free list, attempt to merge it with it's neighbors
         */
        void AttemptCoalesceBlock(FreeBlock* block);

        /** The free list of allocation blocks */
        FreeBlock* m_FreeBlocks;

        /** Pointer to the start of the allocator's memory block */
        void* m_BufferStart;

        /** Pointer to the end of the allocator's memory block */
        void* m_BufferEnd;

        /** Structure used to track and report the state of this allocator */
        AllocatorStatus m_AllocatorStats;
    };
} // namespace sj
