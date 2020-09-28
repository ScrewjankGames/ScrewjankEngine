// STD Headers

// Library Headers

// Screwjank Headers
#include "core/Assert.hpp"
#include "system/allocators/UnmanagedAllocator.hpp"

namespace sj {
    UnmanagedAllocator::UnmanagedAllocator() : m_ActiveAllocationCount(0)
    {
    }

    UnmanagedAllocator::~UnmanagedAllocator()
    {
        SJ_ASSERT(m_ActiveAllocationCount == 0,
                  "Memory leak detected in unmanaged allocator. Check your raw new and deletes?");
    }

    void* UnmanagedAllocator::Allocate(const size_t size, const size_t alignment)
    {
        void* memory = malloc(size);
        m_ActiveAllocationCount++;

        return memory;
    }

    void UnmanagedAllocator::Free(void* memory)
    {
        SJ_ASSERT(memory != nullptr, "Cannot free nullptr");
        free(memory);
        m_ActiveAllocationCount--;
    }
} // namespace sj
