// Parent Include
#include <ScrewjankEngine/system/memory/allocators/FreeListAllocator.hpp>

// Screwjank Headers
#include <ScrewjankShared/utils/MemUtils.hpp>
#include <ScrewjankShared/utils/Assert.hpp>

// STD Includes
#include <cstring>
#include <print>

namespace sj {
    FreeListAllocator::FreeListAllocator()
        : m_FreeBlocks(nullptr), m_BufferStart(nullptr), m_BufferEnd(nullptr)
    {

    }

    FreeListAllocator::FreeListAllocator(size_t buffer_size, void* memory)
        : FreeListAllocator()
    {
        Init(buffer_size, memory);
    }

    FreeListAllocator::FreeListAllocator(FreeListAllocator&& other) noexcept 
    {
        m_FreeBlocks = other.m_FreeBlocks;
        m_BufferStart = other.m_BufferStart;
        m_BufferEnd = other.m_BufferEnd;
        m_AllocatorStats = other.m_AllocatorStats;

        other.m_FreeBlocks = nullptr;
        other.m_BufferStart = nullptr;
        other.m_AllocatorStats = {};
    }

    FreeListAllocator::~FreeListAllocator()
    {
        SJ_ASSERT(m_AllocatorStats.ActiveAllocationCount == 0,
                  "Memory leak detected in FreeListAllocator!");
    }
    
    void FreeListAllocator::Init(size_t buffer_size, void* memory) 
    {
        SJ_ASSERT(!IsInitialized(),
                  "Double initialization of free list allocator detected");

        SJ_ASSERT(buffer_size > sizeof(FreeBlock) && buffer_size > sizeof(AllocationHeader),
                  "FreeListAllocator is not large enough to hold data");

        // Allocate memory from backing allocator
        m_BufferStart = memory;
        m_BufferEnd = (void*)(uintptr_t(memory) + buffer_size);

        // Initialize free list to be a single block of buffer_size situated at start of buffer
        FreeBlock* initial_block = new (m_BufferStart) FreeBlock(buffer_size);
        AddFreeBlock(initial_block);
    }

    void* FreeListAllocator::Allocate(const size_t size, const size_t alignment)
    {
        SJ_ASSERT(IsInitialized(), "Trying to allocate with uninitialized allocator");

        // Search the free list, and return the most suitable free block and the padding required to
        // use it
        std::pair<FreeBlock*, size_t> free_list_search_result =
            FindFreeBlock(size, alignment);

        FreeBlock* const best_fit_block = free_list_search_result.first;
        const size_t header_padding = free_list_search_result.second;

        // If no best fit block was found, halt program
        SJ_ASSERT(best_fit_block != nullptr, "Free list allocator is out of memory.");

        // Remove block from the free list
        RemoveFreeBlock(best_fit_block);

        // Get a copy of the current block's info before it is overwritten
        FreeBlock old_block_info = *best_fit_block;

        // Calculate allocation information
        void* const header_address = (void*)((uintptr_t)best_fit_block + header_padding);
        void* const payload_address = (void*)((uintptr_t)header_address + sizeof(AllocationHeader));

        // Amount of space left in the block for the payload
        // !!!This operation overwrites the data pointed to by best_fit_block!!!
        auto payload_space = old_block_info.Size - sizeof(AllocationHeader) - header_padding;

        SJ_ASSERT(IsMemoryAligned(header_address, alignof(AllocationHeader)),
                  "Allocation header memory is misaligned");

        SJ_ASSERT(IsMemoryAligned(payload_address, alignment),
                  "Allocation payload memory is misaligned");

        // Place the header into memory just before the user's data
        auto header = new (header_address) AllocationHeader(header_padding, payload_space);

        // Calculate unused space and attempt to make a new free block
        uintptr_t payload_end = (uintptr_t)payload_address + size;
        uintptr_t block_end = uintptr_t(payload_address) + payload_space;

        // Padding required to align a new free block after the end of the user payload
        auto new_block_adjustment = GetAlignmentAdjustment(alignof(FreeBlock), (void*)payload_end);

        // Padding bytes for the new block would be left at the end of the current block
        auto unused_space = block_end - (payload_end + new_block_adjustment);

        if (unused_space >= kMinBlockSize) {
            // Remove unused_space from the allocation header's representation of the block
            header->Size -= unused_space;
            SJ_ASSERT(header->Size > 0, "Shit");
            // Place a FreeBlock into the buffer after the user's payload
            void* new_block_address = (void*)(payload_end + new_block_adjustment);
            FreeBlock* new_block = new (new_block_address) FreeBlock(unused_space);

            // Insert the new block into the free list
            AddFreeBlock(new_block);
        }

        m_AllocatorStats.TotalAllocationCount++;
        m_AllocatorStats.TotalBytesAllocated += header->Size;
        m_AllocatorStats.ActiveAllocationCount++;
        m_AllocatorStats.ActiveBytesAllocated += header->Size;

        return payload_address;
    }

    void* FreeListAllocator::Reallocate(void* originalPtr, const size_t size, const size_t alignment)
    {
        // TODO: Would be better to try to grow the current block
        // and avoid the memcpy but this was easier
        AllocationHeader* header = GetAllocationHeader(originalPtr);
        
        void* newPtr = Allocate(size, alignment);
        memcpy(newPtr, originalPtr, std::min(size, header->Size));
        Free(originalPtr);

        size_t bytes = 0;
        FreeBlock* currBlock = m_FreeBlocks;
        while(currBlock)
        {
            bytes += currBlock->Size;
            currBlock = currBlock->Next;
        }

        SJ_ASSERT(bytes > (((uintptr_t)m_BufferEnd - (uintptr_t)m_BufferStart) / 4.0), "nooo");


        return newPtr;
    }

    void FreeListAllocator::Free(void* memory)
    {
        SJ_ASSERT(IsInitialized(), "Trying to deallocate with uninitialized allocator");
        SJ_ASSERT(memory != nullptr, "Cannot free nullptr");
        SJ_ASSERT(IsMemoryInRange(memory), "Pointer is not managed by this allocator!");

        AllocationHeader* block_header = GetAllocationHeader(memory);
        SJ_ASSERT(block_header->MagicHeader == AllocationHeader::kMagicHeader, "Free list allocator corruption detected");

        // Extract header info
        auto block_size = block_header->Padding + block_header->Size + sizeof(AllocationHeader);
        void* block_start = (void*)((uintptr_t)block_header - block_header->Padding);

        SJ_ASSERT(IsMemoryAligned(block_start, alignof(FreeBlock)), "Free block is mis-aligned");

        m_AllocatorStats.ActiveAllocationCount--;
        m_AllocatorStats.ActiveBytesAllocated -= block_header->Size;

        FreeBlock* new_block = new (block_start) FreeBlock(block_size);

        AddFreeBlock(new_block);
    }

    bool FreeListAllocator::IsInitialized() const
    {
        return m_BufferStart != nullptr;
    }

    bool FreeListAllocator::IsMemoryInRange(void* memory) const
    {
        return uintptr_t(memory) >= uintptr_t(m_BufferStart) &&
               uintptr_t(memory) <= uintptr_t(m_BufferEnd);
    }

    uintptr_t FreeListAllocator::Begin() const
    {
        return reinterpret_cast<uintptr_t>(m_BufferStart);
    }

    uintptr_t FreeListAllocator::End() const
    {
        return reinterpret_cast<uintptr_t>(m_BufferEnd);
    }

    std::pair<FreeListAllocator::FreeBlock*, size_t>
    FreeListAllocator::FindFreeBlock(const size_t size, const size_t alignment)
    {
        // TODO (MrLever): Replace linear search with a Red-Black tree

        const auto header_and_payload_size = size + sizeof(AllocationHeader);

        // When looking to align memory, align by the strictest alignment requirement. This will
        // allow header and payload to be packed together in memory without padding between them,
        // only padding in the allocation is placed before the header.
        const size_t alignment_requirement = std::max(alignof(AllocationHeader), alignment);

        // Iterator for free list
        FreeBlock* curr_block = m_FreeBlocks;

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
                return {curr_block, required_padding};
            }

            curr_block = curr_block->Next;
        }

        return {nullptr, 0};
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

            curr_block = curr_block->Next;
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
            // Rewire the head of the list
            m_FreeBlocks = block->Next;
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
} // namespace sj
