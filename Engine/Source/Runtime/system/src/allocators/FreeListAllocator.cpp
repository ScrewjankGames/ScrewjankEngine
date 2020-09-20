// STD Headers

// Library Headers

// Screwjank Headers
#include "system/Memory.hpp"
#include "system/allocators/FreeListAllocator.hpp"

namespace Screwjank {
    FreeListAllocator::FreeListAllocator(size_t buffer_size,
                                         Allocator* backing_allocator,
                                         const char* debug_name)
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
                "FreeListAllocator {} has insufficient memory to perform allocation of {} bytes",
                m_DebugName,
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
        header->Size = size;
        header->Padding = header_padding;

        // Calculate the memory location returned to the user
        void* payload_loc = (void*)((uintptr_t)header_loc + sizeof(AllocationHeader));

        // Assert payload_loc satisfy's the user's alignment request
        SJ_ASSERT(IsMemoryAligned(payload_loc, alignment),
                  "Allocation payload memory is misaligned");

        // Determine how much space in the block is unused after allocation
        uintptr_t payload_end = (uintptr_t)payload_loc + size;
        uintptr_t block_end = uintptr_t(best_fit_block) + old_block_info.Size;
        auto unused_space = block_end - payload_end;

        // If the remaining space is big enough to host a free block and later an allocation, create
        // a new free block out of it
        if (unused_space > sizeof(FreeBlock) && unused_space > sizeof(AllocationHeader)) {
            // payload_end is the first byte **AFTER** the user's data
            FreeBlock* new_block = new ((void*)payload_end) FreeBlock(unused_space);
            AddFreeBlock(new_block);
        }

        m_MemoryStats.TotalAllocationCount++;
        m_MemoryStats.TotalBytesAllocated += size;
        m_MemoryStats.ActiveAllocationCount++;
        m_MemoryStats.ActiveBytesAllocated += size;
        return payload_loc;
    }

    void FreeListAllocator::Free(void* memory)
    {
        // Allocation Header
        AllocationHeader* block_header =
            (AllocationHeader*)((uintptr_t)memory - sizeof(AllocationHeader));

        // Extract header info
        auto block_size = block_header->Padding + block_header->Size + sizeof(AllocationHeader);
        void* block_start = (void*)((uintptr_t)block_header - block_header->Padding);

        SJ_ASSERT(IsMemoryAligned(block_start, alignof(FreeBlock)), "Free block is mis-aligned");

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
                GetAlignmentOffset(alignment_requirement, fist_possible_payload_address);
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
        if (m_FreeBlocks == nullptr) {
            m_FreeBlocks = new_block;
            return;
        }

        // New block needs to be inserted to the front of the free list
        if ((uintptr_t)new_block < (uintptr_t)m_FreeBlocks) {
            new_block->Next = m_FreeBlocks;
            m_FreeBlocks->Previous = new_block;
            m_FreeBlocks = new_block;
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
                return;
            }
        }

        // If no suitable place between two blocks was found, tack the block onto the end of the
        // free list
        curr_block->Next = new_block;
        new_block->Previous = curr_block;
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

} // namespace Screwjank
