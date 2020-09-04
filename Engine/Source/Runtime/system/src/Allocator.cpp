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

    void Allocator::Log()
    {
        SJ_LOG_INFO("No memory stats available for requested allocator");
    }
} // namespace Screwjank
