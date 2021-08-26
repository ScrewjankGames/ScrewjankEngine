// STD Headers

// Library Headers

// Screwjank Headers
#include <core/Assert.hpp>
#include <system/allocators/ProxyAllocator.hpp>

namespace sj {
    ProxyAllocator::ProxyAllocator(Allocator* backing_allocator, const char* debug_name)
        : m_BackingAllocator(backing_allocator), m_Name(debug_name)
    {
    }

    ProxyAllocator::~ProxyAllocator()
    {
        SJ_ASSERT(m_AllocatorStats.ActiveAllocationCount == 0,
                  "Memory leak detected in proxy allocator");
    }

    void* ProxyAllocator::Allocate(const size_t size, const size_t alignment)
    {
        m_AllocatorStats.TotalAllocationCount++;
        m_AllocatorStats.ActiveAllocationCount++;
        m_AllocatorStats.TotalBytesAllocated += size;
        return m_BackingAllocator->Allocate(size, alignment);
    }

    void ProxyAllocator::Free(void* memory)
    {
        SJ_ASSERT(memory != nullptr, "Cannot free nullptr");

        m_AllocatorStats.ActiveAllocationCount--;
        m_BackingAllocator->Free(memory);
    }
} // namespace sj
