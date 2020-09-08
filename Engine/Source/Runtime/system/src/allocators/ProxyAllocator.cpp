// STD Headers

// Library Headers

// Screwjank Headers
#include "core/Assert.hpp"
#include "system/allocators/ProxyAllocator.hpp"

namespace Screwjank {
    ProxyAllocator::ProxyAllocator(Allocator* backing_allocator, const char* debug_name)
        : Allocator(backing_allocator, debug_name), m_BackingAllocator(backing_allocator)
    {
    }

    ProxyAllocator::~ProxyAllocator()
    {
        SJ_ASSERT(m_MemoryStats.ActiveAllocationCount == 0,
                  "Memory leak detected in proxy allocator");
    }

    void* ProxyAllocator::Allocate(const size_t size, const size_t alignment)
    {
        m_MemoryStats.TotalAllocationCount++;
        m_MemoryStats.ActiveAllocationCount++;
        m_MemoryStats.TotalBytesAllocated += size;
        return m_BackingAllocator->Allocate(size, alignment);
    }

    void ProxyAllocator::Free(void* memory)
    {
        m_MemoryStats.ActiveAllocationCount--;
        m_BackingAllocator->Free(memory);
    }
} // namespace Screwjank
