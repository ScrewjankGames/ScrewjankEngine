// STD Headers
#include <memory>

// Screwjank Headers
#include "core/Log.hpp"
#include "system/Allocator.hpp"

namespace Screwjank {
    Allocator::Allocator(Allocator* backing_allocator, const char* debug_name)
        : m_DebugName(debug_name)
    {
    }
} // namespace Screwjank
