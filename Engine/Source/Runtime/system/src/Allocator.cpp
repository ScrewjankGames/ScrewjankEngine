// STD Headers
#include <memory>

// Screwjank Headers
#include "core/Log.hpp"
#include "system/Allocator.hpp"

namespace Screwjank {

    void* BasicAllocator::Allocate(const size_t size, const size_t alignment)
    {
        void* memory = malloc(size);
        return memory;
    }

    void BasicAllocator::Free(void* memory)
    {
        free(memory);
    }

    Allocator::Allocator(const char* debug_name) : m_DebugName(debug_name)
    {
    }
} // namespace Screwjank
