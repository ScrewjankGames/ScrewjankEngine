// STD Headers

// Library Headers

// Screwjank Headers
#include "system/Memory.hpp"
#include "system/allocators/FreeListAllocator.hpp"

namespace Screwjank {
    FreeListAllocator::FreeListAllocator(size_t buffer_size, Allocator* backing_allocator)
        : m_BackingAllocator(backing_allocator), m_FreeBlocks(nullptr)
    {
        SJ_ASSERT(buffer_size > sizeof(FreeBlock) && buffer_size > sizeof(AllocationHeader),
                  "FreeListAllocator is not large enough to hold data");

        // Allocate memory from backing allocator
        m_BufferStart = m_BackingAllocator->Allocate(buffer_size);

        // Initialize free list to be a single block of buffer_size situated at start of buffer
        FreeBlock* initial_block = new (m_BufferStart) FreeBlock(buffer_size);
        AddFreeBlock(initial_block);
    }

    FreeListAllocator::~FreeListAllocator()
    {
        SJ_ASSERT(m_AllocatorStats.ActiveAllocationCount == 0,
                  "Memory leak detected in FreeListAllocator!");
        m_BackingAllocator->Free(m_BufferStart);
    }

    void* FreeListAllocator::Allocate(const size_t size, const size_t alignment)
    {
        // Search the free list, and return the most suitable free block and the padding required to
        // use it
        auto free_list_search_result = FindFreeBlock(size, alignment);
        FreeBlock* best_fit_block = free_list_search_result.first;
        size_t header_padding = free_list_search_result.second;

        // If no best fit block was found, log an error
        if (best_fit_block == nullptr) {
            SJ_ENGINE_LOG_ERROR(
                "FreeListAllocator has insufficient memory to perform allocation of {} bytes",
                size);
            return nullptr;
        }

        // We're going to use the block, remove it from the free list
        RemoveFreeBlock(best_fit_block);

        // Get a copy of the current block's info before it is overwritten in the buffer
        FreeBlock old_block_info = *best_fit_block;

        // Place allocation header in the best fit block
        void* header_loc = (void*)((uintptr_t)best_fit_block + header_padding);

        // Assert that the header is being placed at an aligned memory address
        SJ_ASSERT(IsMemoryAligned(header_loc, alignof(AllocationHeader)),
                  "Allocation header memory is misaligned");

        // Place the header into memory just before the user's data
        auto header = new (header_loc) AllocationHeader();
        header->Padding = header_padding;
        header->Size = old_block_info.Size - sizeof(AllocationHeader) - header_padding;

        // Calculate the memory location returned to the user
        void* payload_loc = (void*)((uintptr_t)header_loc + sizeof(AllocationHeader));

        // Assert payload_loc satisfy's the user's alignment request
        SJ_ASSERT(IsMemoryAligned(payload_loc, alignment),
                  "Allocation payload memory is misaligned");

        // See if we can fit a memory-aligned free block after the end of the payload in this
        // current block
        uintptr_t payload_end = (uintptr_t)payload_loc + size;
        uintptr_t block_end = uintptr_t(best_fit_block) + old_block_info.Size;

        // Find the amount of padding we need after the payload to align a new free block
        auto new_block_adjustment = GetAlignmentAdjustment(alignof(FreeBlock), (void*)payload_end);

        auto unused_space = block_end - (payload_end + new_block_adjustment);

        // If the remaining space is big enough to host a free block and later an allocation, create
        // a new free block out of it
        if (unused_space > sizeof(FreeBlock) && unused_space > sizeof(AllocationHeader)) {
            // Remove unused_space from the allocation header's representation of the block
            header->Size -= unused_space;

            void* new_block_location = (void*)(payload_end + new_block_adjustment);

            SJ_ASSERT(IsMemoryAligned(new_block_location, alignof(FreeBlock)),
                      "New freeblock improperly aligned.");

            // payload_end is the first byte **AFTER** the user's data
            FreeBlock* new_block = new (new_block_location) FreeBlock(unused_space);
            AddFreeBlock(new_block);
        }

        m_AllocatorStats.TotalAllocationCount++;
        m_AllocatorStats.TotalBytesAllocated += header->Size;
        m_AllocatorStats.ActiveAllocationCount++;
        m_AllocatorStats.ActiveBytesAllocated += header->Size;
        return payload_loc;
    }

    void FreeListAllocator::Free(void* memory)
    {
        SJ_ASSERT(memory != nullptr, "Cannot free nullptr");

        // Allocation Header
        AllocationHeader* block_header =
            (AllocationHeader*)((uintptr_t)memory - sizeof(AllocationHeader));

        // Extract header info
        auto block_size = block_header->Padding + block_header->Size + sizeof(AllocationHeader);
        void* block_start = (void*)((uintptr_t)block_header - block_header->Padding);

        SJ_ASSERT(IsMemoryAligned(block_start, alignof(FreeBlock)), "Free block is mis-aligned");

        m_AllocatorStats.ActiveAllocationCount--;
        m_AllocatorStats.ActiveBytesAllocated -= block_header->Size;

        // Potentially stomps on memory useb by block_header!
        FreeBlock* new_block = new (block_start) FreeBlock(block_size);

        AddFreeBlock(new_block);
    }

    std::pair<FreeListAllocator::FreeBlock*, size_t>
    FreeListAllocator::FindFreeBlock(const size_t size, const size_t alignment)
    {
        // TODO (MrLever): Replace linear search with a Red-Black tree

        const auto header_and_payload_size = size + sizeof(AllocationHeader);

        // When looking to align memory, align by the most strict alignment requirement. This will
        // allow header and payload to be packed together in memory without padding between them,
        // only padding in the allocation is placed before the header.
        const size_t alignment_requirement = std::max(alignof(AllocationHeader), alignment);

        // Iterator for free list
        FreeBlock* curr_block = m_FreeBlocks;

        // Storage for results of the free-list search
        FreeBlock* best_fit_block = nullptr;
        size_t header_padding = 0;

        // Search the free list for a best-fit block
        while (curr_block != nullptr) {

            // When allocating, we must calculate alignment from this address to leave room for the
            // allocation header
            void* fist_possible_payload_address =
                (void*)((uintptr_t)curr_block + sizeof(AllocationHeader));

            // Get the padding needed to align payload from first possible playload addresss
            size_t required_padding =
                GetAlignmentAdjustment(alignment_requirement, fist_possible_payload_address);
            size_t total_allocation_size = header_and_payload_size + required_padding;

            // If the current free block is large enough to support allocation
            if (total_allocation_size <= curr_block->Size) {
                // Determine if it is a better fit than the current best-fit block
                if (best_fit_block == nullptr || curr_block->Size < best_fit_block->Size) {
                    // This is the first block found that can handle the allocation
                    best_fit_block = curr_block;
                    header_padding = required_padding;
                }
            }

            curr_block = curr_block->Next;
        }

        return {best_fit_block, header_padding};
    }

    void FreeListAllocator::AddFreeBlock(FreeBlock* new_block)
    {
        // If there no free list, new_block becomes the new head
        if (m_FreeBlocks == nullptr) {
            m_FreeBlocks = new_block;
            return;
        }

        // New block needs to be inserted to the front of the free list
        if ((uintptr_t)new_block < (uintptr_t)m_FreeBlocks) {
            new_block->Next = m_FreeBlocks;
            m_FreeBlocks->Previous = new_block;
            m_FreeBlocks = new_block;
            AttemptCoalesceBlock(new_block);
            return;
        }

        // Otherwise, iterate the list and find a place to add the free block
        auto curr_block = m_FreeBlocks;

        // Insert the free block into the free list sorted by memory address to avoid poor cache
        // performance
        while (curr_block->Next != nullptr) {
            if ((uintptr_t)new_block > (uintptr_t)curr_block &&
                (uintptr_t)new_block < (uintptr_t)(curr_block->Next)) {

                new_block->Next = curr_block->Next;
                curr_block->Next->Previous = new_block;
                new_block->Previous = curr_block;
                curr_block->Next = new_block;
                AttemptCoalesceBlock(new_block);
                return;
            }
        }

        // If no suitable place between two blocks was found, tack the block onto the end of the
        // free list
        curr_block->Next = new_block;
        new_block->Previous = curr_block;
        AttemptCoalesceBlock(new_block);
    }

    void FreeListAllocator::RemoveFreeBlock(FreeBlock* block)
    {
        SJ_ASSERT(block != nullptr, "Cannot free null block");

        // If the block being removed is the head of the list
        if (block == m_FreeBlocks) {
            // Rewiring is not sufficient, explicitly clear the head of the list
            m_FreeBlocks = nullptr;
            return;
        }

        // Rewire linked list links to no longer visit block
        if (block->Previous != nullptr) {
            block->Previous->Next = block->Next;
        }

        if (block->Next != nullptr) {
            block->Next->Previous = block->Previous;
        }
    }

    void FreeListAllocator::AttemptCoalesceBlock(FreeBlock* block)
    {
        // Attempt to coalesce with left neighbor, moving block pointer back if necessary
        if (block->Previous != nullptr) {
            uintptr_t left_end = (uintptr_t)block->Previous + block->Previous->Size;

            // If this block starts exactly where the previous block ends, coalesce
            if ((uintptr_t)block == left_end) {
                block->Previous->Next = block->Next;
                if (block->Next != nullptr) {
                    block->Next->Previous = block->Previous;
                }

                block->Previous->Size += block->Size;
                block = block->Previous;
            }
        }

        // Attempt to coalesce with right neighbor
        if (block->Next != nullptr) {
            uintptr_t block_end = (uintptr_t)block + block->Size;

            // If the end of this block is the start of the next block, coalesce
            if (block_end == (uintptr_t)block->Next) {
                block->Size += block->Next->Size;
                block->Next = block->Next->Next;
                if (block->Next != nullptr) {
                    block->Next->Previous = block;
                }
            }
        }
    }

} // namespace Screwjank
