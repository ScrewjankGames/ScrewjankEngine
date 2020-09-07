// STD Headers

// Library Headers

// Engine Headers
#include "core/Assert.hpp"
#include "core/Log.hpp"
#include "system/allocators/StackAllocator.hpp"
#include "system/Memory.hpp"

namespace Screwjank {

    StackAllocator::StackAllocator(size_t buffer_size,
                                   Allocator* backing_allocator,
                                   const char* debug_name)
        : Allocator(backing_allocator, debug_name), m_BackingAllocator(backing_allocator),
          m_CurrentHeader(nullptr), m_Offset(nullptr), m_Capacity(buffer_size)
    {
        m_BufferStart = m_BackingAllocator->Allocate(buffer_size);
        m_Offset = m_BufferStart;
    }

    StackAllocator::~StackAllocator()
    {
        SJ_ASSERT(m_Offset == m_BufferStart, "Memory leak detected in memory allocator");

        m_BackingAllocator->Free(m_BufferStart);
    }

    void* StackAllocator::Allocate(const size_t size, const size_t alignment)
    {
        auto free_space = m_Capacity - (uintptr_t(m_Offset) - uintptr_t(m_BufferStart));

        if (free_space < size) {
            SJ_LOG_ERROR("Allocator {} has insufficient memory to perform requested allocation.",
                         m_DebugName);
            return nullptr;
        }

        // Do pointer math to create a new allocation header
        uintptr_t header_offset = GetAlignmentOffset(alignof(StackAllocatorHeader), m_Offset);

        // Calculate address from which the allocation will be aligned
        uintptr_t allocation_start =
            (uintptr_t)m_Offset + header_offset + sizeof(StackAllocatorHeader);

        // Calculate number of bytes needed to align allocation from alocation_start
        uintptr_t allocation_offset = GetAlignmentOffset(alignment, (void*)allocation_start);

        // Cacluate total space needed for header, allocation, and padding
        uintptr_t total_allocation_size =
            header_offset + sizeof(StackAllocatorHeader) + allocation_offset + size;

        if (free_space < total_allocation_size) {
            SJ_LOG_ERROR("Allocator {} has insufficient memory to perform requested allocation.",
                         m_DebugName);
            return nullptr;
        }

        // Allocate the header
        auto header_memory = (uintptr_t)m_Offset + header_offset;
        auto old_header = m_CurrentHeader;
        m_CurrentHeader =
            new ((void*)header_memory) StackAllocatorHeader {old_header, header_offset};

        // Update m_offset
        m_Offset = (void*)(uintptr_t(m_Offset) + total_allocation_size);

        // Return pointer to the start of the allocation
        return (void*)(allocation_start + allocation_offset);
    }

    void StackAllocator::Free(void* memory)
    {
        SJ_ASSERT(m_CurrentHeader != nullptr, "You cannot free from an empty stack allocator");

        // The memory argument is ignored unless it is a specific value
        if (memory != nullptr) {
            SJ_ASSERT((uintptr_t)memory ==
                          (uintptr_t)m_CurrentHeader + sizeof(StackAllocatorHeader),
                      "Stack allocator cannot free memory that is not on top of the stack.");
        }

        // Get beginning of current frame
        auto current_frame_start = (uintptr_t)m_CurrentHeader - m_CurrentHeader->HeaderOffset;

        // Roll free memory pointer back to start of current frame
        m_Offset = (void*)current_frame_start;

        // Roll header back to previous header
        m_CurrentHeader = m_CurrentHeader->PreviousHeader;

        return;
    }

    void* StackAllocator::Push(size_t size, size_t alignment)
    {
        return Allocate(size, alignment);
    }

    void StackAllocator::Pop()
    {
        Free(nullptr);
        return;
    }

} // namespace Screwjank
