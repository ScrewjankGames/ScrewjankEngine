// STD Headers

// Library Headers

// Screwjank Headers
#include "core/Assert.hpp"
#include "system/allocators/LinearAllocator.hpp"
#include "system/Memory.hpp"

namespace Screwjank {
    LinearAllocator::LinearAllocator(size_t buffer_size,
                                     Allocator* backing_allocator,
                                     const char* debug_name)
        : Allocator(debug_name), m_BackingAllocator(backing_allocator)
    {
        m_MemoryStats.Capacity = buffer_size;
        m_BufferStart = m_BackingAllocator->Allocate(buffer_size);
        m_CurrFrameStart = m_BufferStart;
    }

    LinearAllocator::~LinearAllocator()
    {
        // While other allocators should assert, linear allocators are perfectly capable of freeing
        // all of their allocations on destruction
        if (!(m_MemoryStats.ActiveAllocationCount == 0 &&
              m_MemoryStats.FreeSpace() == m_MemoryStats.Capacity)) {
            SJ_LOG_WARN("Linear allocator {} was not properly reset before destruction.");
            Reset();
        }

        m_BackingAllocator->Free(m_BufferStart);
    }

    void* LinearAllocator::Allocate(const size_t size, const size_t alignment)
    {
        // Ensure there is enough space to satisfy allocation
        if (m_MemoryStats.FreeSpace() < size + GetAlignmentOffset(alignment, m_CurrFrameStart)) {
            SJ_LOG_ERROR("{} has insufficient memory to perform requested allocation", m_DebugName);
            return nullptr;
        }

        auto allocated_memory =
            AlignMemory(alignment, size, m_CurrFrameStart, m_MemoryStats.FreeSpace());

        // Track allocation data
        m_MemoryStats.ActiveAllocationCount++;
        m_MemoryStats.TotalAllocationCount++;

        auto num_bytes_allocated = size + GetAlignmentOffset(alignment, m_CurrFrameStart);
        m_MemoryStats.TotalBytesAllocated += num_bytes_allocated;
        m_MemoryStats.ActiveBytesAllocated += num_bytes_allocated;

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
        m_MemoryStats.ActiveAllocationCount = 0;
        m_MemoryStats.ActiveBytesAllocated = 0;
        m_CurrFrameStart = m_BufferStart;
    }
} // namespace Screwjank
