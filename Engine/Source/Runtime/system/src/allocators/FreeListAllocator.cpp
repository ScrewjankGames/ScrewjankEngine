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
        FreeBlock* best_fit_block = nullptr;
        FreeBlock* curr_block = m_FreeBlocks;

        while (curr_block != nullptr) {
            size_t required_padding = GetAlignmentOffset(alignment, (void*)curr_block);
            size_t total_allocation_size = size + required_padding;

            // If the current free block is large enough to support allocation
            if (total_allocation_size < curr_block->Size) {

                // Determine if it is a better fit than the current best-fit block
                if (best_fit_block == nullptr) {
                    // This is the first block found that can handle the allocation
                    best_fit_block = curr_block;
                } else if (curr_block->Size < best_fit_block->Size) {
                    // This block is better than current best-fit block
                    best_fit_block = curr_block;
                }
            }

            curr_block = curr_block->Next;
        }

        // If no best fit block was found, log an error
        if (best_fit_block == nullptr) {
            SJ_ENGINE_LOG_ERROR(
                "FreeListAllocator {} has insufficient memory to perform allocation of {} bytes",
                m_DebugName,
                size);
            return nullptr;
        }

        // Allocate in best fit block, splitting off new free block if appropriate
        return nullptr;
    }

} // namespace Screwjank
