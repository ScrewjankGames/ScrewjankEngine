// STD Headers

// Library Headers

// Screwjank Headers
#include "core/Assert.hpp"
#include "system/allocators/UnmanagedAllocator.hpp"

namespace Screwjank {
    UnmanagedAllocator::UnmanagedAllocator() : m_NumAllocations(0)
    {
        m_DebugName = "System Allocator";
    }

    UnmanagedAllocator::~UnmanagedAllocator()
    {
        SJ_ASSERT(m_NumAllocations == 0,
                  "Memory leak detected in system allocator. Check your raw new and deletes");
    }

    void* UnmanagedAllocator::Allocate(const size_t size, const size_t alignment)
    {
        void* memory = malloc(size);
        m_NumAllocations++;

        return memory;
    }

    void UnmanagedAllocator::Free(void* memory)
    {
        free(memory);
        m_NumAllocations--;
    }
} // namespace Screwjank
