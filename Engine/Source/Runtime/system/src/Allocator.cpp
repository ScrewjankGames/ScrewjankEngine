// STD Headers
#include <memory>

// Screwjank Headers
#include "core/Log.hpp"
#include "system/Allocator.hpp"

namespace Screwjank {
    Allocator::Allocator(const char* debug_name) : m_DebugName(debug_name), m_MemoryStats {}
    {
    }
} // namespace Screwjank
