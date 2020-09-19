// STD Headers

// Library Headers

// Screwjank Headers
#include "system/Memory.hpp"
#include "system/allocators/FreeListAllocator.hpp"

namespace Screwjank {
    FreeListAllocator::FreeListAllocator(size_t buffer_size,
                                         Allocator* backing_allocator,
                                         const char* debug_name)
        : m_BackingAllocator(backing_allocator)
    {
        SJ_ASSERT(buffer_size > sizeof(FreeBlock),
                  "FreeListAllocator is too small to hold any data");

        m_BufferStart = m_BackingAllocator->Allocate(buffer_size);

        // Initialize free list to be a single block of buffer_size situated at start of buffer
        m_FreeBlocks = new (m_BufferStart) FreeBlock {buffer_size, nullptr};
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

        // Get a copy of the current block's info before it is stomped by allocation
        FreeBlock block_info = *best_fit_block;

        // Place allocation header in the best fit block
        void* header_loc = (void*)((uintptr_t)best_fit_block + header_padding);

        SJ_ASSERT(IsMemoryAligned(header_loc, alignof(AllocationHeader)),
                  "Allocation header memory is misaligned");

        auto header = new (header_loc) AllocationHeader();
        header->Size = size;
        header->Padding = header_padding;

        // Calculate location of memory returned to the user
        void* payload_loc = (void*)((uintptr_t)header_loc + sizeof(AllocationHeader));
        SJ_ASSERT(IsMemoryAligned(payload_loc, alignment),
                  "Allocation payload memory is misaligned");

        // Determine how much space in the block is unused after allocation
        uintptr_t payload_end = (uintptr_t)payload_loc + size;
        uintptr_t block_end = uintptr_t(best_fit_block) + block_info.Size;

        auto unused_space = block_end - payload_end;

        // If the remaining space is big enough to host a free block and later an allocation, create
        // a new free block out of it
        if (unused_space > sizeof(FreeBlock) && unused_space > sizeof(AllocationHeader)) {
            // TODO (MrLever) create new free blocks please.
        }

        m_MemoryStats.TotalAllocationCount++;
        m_MemoryStats.TotalBytesAllocated += size;
        m_MemoryStats.ActiveAllocationCount++;
        m_MemoryStats.ActiveBytesAllocated += size;
        return payload_loc;
    }

    void FreeListAllocator::Free(void* memory)
    {
        SJ_ASSERT(false, "Not yet implemented");
    }

    std::pair<FreeListAllocator::FreeBlock*, size_t>
    FreeListAllocator::FindFreeBlock(const size_t size, const size_t alignment)
    {
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
            if (total_allocation_size < curr_block->Size) {
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

    void FreeListAllocator::RegisterNewFreeBlock(FreeBlock* block)
    {
        if (m_FreeBlocks == nullptr) {
            m_FreeBlocks = block;
            return;
        }

        auto curr_block = m_FreeBlocks;

        // Insert the free block into the free list sorted by memory address to avoid poor cache
        // performance
        while (curr_block->Next != nullptr) {
            if ((uintptr_t)block > (uintptr_t)curr_block &&
                (uintptr_t)block < (uintptr_t)(curr_block->Next)) {

                block->Next = curr_block->Next;
                curr_block->Next->Previous = block;
                block->Previous = curr_block;
                curr_block->Next = block;
                return;
            }
        }

        curr_block->Next = block;
        block->Previous = curr_block;
    }

} // namespace Screwjank
