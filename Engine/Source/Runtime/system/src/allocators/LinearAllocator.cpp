// STD Headers

// Library Headers

// Screwjank Headers
#include "core/Assert.hpp"
#include "system/Memory.hpp"
#include "system/allocators/LinearAllocator.hpp"

namespace sj {
    LinearAllocator::LinearAllocator(size_t buffer_size, Allocator* backing_allocator)
        : m_BackingAllocator(backing_allocator)
    {
        m_AllocatorStats.Capacity = buffer_size;
        m_BufferStart = m_BackingAllocator->Allocate(buffer_size);
        m_CurrFrameStart = m_BufferStart;
    }

    LinearAllocator::~LinearAllocator()
    {
        // While other allocators should assert, linear allocators are perfectly capable of freeing
        // all of their allocations on destruction
        if (!(m_AllocatorStats.ActiveAllocationCount == 0 &&
              m_AllocatorStats.FreeSpace() == m_AllocatorStats.Capacity)) {
            SJ_ENGINE_LOG_WARN("Linear allocator was not properly reset before destruction.");
            Reset();
        }

        m_BackingAllocator->Free(m_BufferStart);
    }

    void* LinearAllocator::Allocate(const size_t size, const size_t alignment)
    {
        // Ensure there is enough space to satisfy allocation
        if (m_AllocatorStats.FreeSpace() < size + GetAlignmentOffset(alignment, m_CurrFrameStart)) {
            SJ_ENGINE_LOG_FATAL("Allocator has insufficient memory to perform requested allocation");
            return nullptr;
        }

        auto allocated_memory =
            AlignMemory(alignment, size, m_CurrFrameStart, m_AllocatorStats.FreeSpace());

        // Track allocation data
        m_AllocatorStats.ActiveAllocationCount++;
        m_AllocatorStats.TotalAllocationCount++;

        auto num_bytes_allocated = size + GetAlignmentOffset(alignment, m_CurrFrameStart);
        m_AllocatorStats.TotalBytesAllocated += num_bytes_allocated;
        m_AllocatorStats.ActiveBytesAllocated += num_bytes_allocated;

        // Bump allocation pointer to the first free byte after the current allocation
        m_CurrFrameStart = (void*)((uintptr_t)allocated_memory + size);

        return allocated_memory;
    }

    void LinearAllocator::Free(void* memory)
    {
        SJ_ASSERT(false, "Linear allocators do not allow memory to be freed.");
    }

    void LinearAllocator::Reset()
    {
        // Track allocation data
        m_AllocatorStats.ActiveAllocationCount = 0;
        m_AllocatorStats.ActiveBytesAllocated = 0;
        m_CurrFrameStart = m_BufferStart;
    }
} // namespace sj
