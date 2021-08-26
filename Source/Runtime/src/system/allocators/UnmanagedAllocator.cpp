// STD Headers

// Library Headers

// Screwjank Headers
#include <core/Assert.hpp>
#include <system/allocators/UnmanagedAllocator.hpp>

namespace sj {
    UnmanagedAllocator::UnmanagedAllocator() : m_ActiveAllocationCount(0)
    {
    }

    UnmanagedAllocator::~UnmanagedAllocator()
    {
    }

    void* UnmanagedAllocator::Allocate(const size_t size, const size_t alignment)
    {
        void* memory = malloc(size);
        return memory;
    }

    void UnmanagedAllocator::Free(void* memory)
    {
        SJ_ASSERT(memory != nullptr, "Cannot free nullptr");
        free(memory);
    }
} // namespace sj
