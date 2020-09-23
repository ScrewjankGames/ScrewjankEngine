// STD Headers

// Library Headers

// Screwjank Headers
#include "core/Assert.hpp"
#include "system/allocators/UnmanagedAllocator.hpp"

namespace Screwjank {
    UnmanagedAllocator::UnmanagedAllocator()
    {
        m_DebugName = "System Allocator";
    }

    UnmanagedAllocator::~UnmanagedAllocator()
    {
        SJ_ASSERT(m_MemoryStats.ActiveAllocationCount == 0,
                  "Memory leak detected in system allocator. Check your raw new and deletes");
    }

    void* UnmanagedAllocator::Allocate(const size_t size, const size_t alignment)
    {
        void* memory = malloc(size);
        m_MemoryStats.ActiveAllocationCount++;

        return memory;
    }

    void UnmanagedAllocator::Free(void* memory)
    {
        SJ_ASSERT(memory != nullptr, "Cannot free nullptr");
        free(memory);
        m_MemoryStats.ActiveAllocationCount--;
    }
} // namespace Screwjank
