// Parent Include
#include <ScrewjankEngine/system/memory/allocators/StackAllocator.hpp>

// STD Headers

// Library Headers

// Engine Headers
#include <ScrewjankEngine/utils/Assert.hpp>
#include <ScrewjankEngine/utils/Log.hpp>
#include <ScrewjankEngine/system/memory/Utils.hpp>

namespace sj {

    StackAllocator::StackAllocator(size_t buffer_size, void* memory)
    {
        Init(buffer_size, memory);
    }

    StackAllocator::~StackAllocator()
    {
        SJ_ASSERT(m_Offset == m_BufferStart, "Memory leak detected in stack allocator");
    }

    void StackAllocator::Init(size_t buffer_size, void* memory)
    {
        m_BufferStart = memory;
        m_Offset = m_BufferStart;
        m_CurrentHeader = nullptr;
        m_Capacity = buffer_size;
    }

    void* StackAllocator::Allocate(const size_t size, const size_t alignment)
    {
        SJ_ASSERT(m_BufferStart != nullptr, "Uninitialized allocator use.");

        // Calculate padding needed to align header and payload
        size_t alignment_requirement = std::max(alignment, alignof(StackAllocatorHeader));

        // Get the padding required to align the header and payload in memory
        void* fist_possible_payload_address =
            (void*)((uintptr_t)m_Offset + sizeof(StackAllocatorHeader));

        size_t required_padding =
            GetAlignmentAdjustment(alignment_requirement, fist_possible_payload_address);

        size_t total_allocation_size = required_padding + sizeof(StackAllocatorHeader) + size;

        // Ensure the allocator has enough memory to satisfy the allocation
        auto free_space = m_Capacity - (uintptr_t(m_Offset) - uintptr_t(m_BufferStart));
        if (free_space < total_allocation_size) {
            SJ_ENGINE_LOG_ERROR(
                "Stack allocator has insufficient memory to perform requested allocation.");
            return nullptr;
        }

        // Allocate the header
        void* header_memory = (void*)((uintptr_t)m_Offset + required_padding);

        SJ_ASSERT(IsMemoryAligned(header_memory, alignof(StackAllocatorHeader)),
                  "Attempting to place allocation header at unaligned address!");

        auto old_header = m_CurrentHeader;

        m_CurrentHeader =
            new ((void*)header_memory) StackAllocatorHeader {old_header, required_padding};

        // Update m_offset
        m_Offset = (void*)(uintptr_t(m_Offset) + total_allocation_size);

        // Return pointer to the start of the allocation
        return (void*)((uintptr_t)header_memory + sizeof(StackAllocatorHeader));
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

        // Get beginning of "top" allocation frame
        auto current_frame_start = (uintptr_t)m_CurrentHeader - m_CurrentHeader->HeaderOffset;

        // Roll free memory pointer back to start of current frame
        m_Offset = (void*)current_frame_start;

        // Roll header back to previous header
        m_CurrentHeader = m_CurrentHeader->PreviousHeader;

        return;
    }

    void* StackAllocator::PushAlloc(size_t size, size_t alignment)
    {
        return Allocate(size, alignment);
    }

    void StackAllocator::PopAlloc()
    {
        Free(nullptr);
        return;
    }

    uintptr_t StackAllocator::Begin() const
    {
        return reinterpret_cast<uintptr_t>(m_BufferStart);
    }

    uintptr_t StackAllocator::End() const
    {
        return reinterpret_cast<uintptr_t>(m_BufferStart) + m_Capacity;
    }

} // namespace sj
