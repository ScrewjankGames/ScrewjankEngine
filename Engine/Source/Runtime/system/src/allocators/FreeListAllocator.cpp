// STD Headers

// Library Headers

// Screwjank Headers
#include "system/allocators/FreeListAllocator.hpp"

namespace Screwjank {
    FreeListAllocator::FreeListAllocator(size_t buffer_size,
                                         Allocator* backing_allocator,
                                         const char* debug_name)
        : m_BackingAllocator(backing_allocator)
    {
        m_BufferStart = m_BackingAllocator->Allocate(buffer_size);
    }

    FreeListAllocator::~FreeListAllocator()
    {
        m_BackingAllocator->Free(m_BufferStart);
    }
} // namespace Screwjank
