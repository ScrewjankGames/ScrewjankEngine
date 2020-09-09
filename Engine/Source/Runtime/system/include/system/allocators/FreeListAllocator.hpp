#pragma once
// STD Headers

// Library Headers

// Screwjank Headers
#include "core/MemorySystem.hpp"
#include "system/Allocator.hpp"

namespace Screwjank {
    class FreeListAllocator : public Allocator
    {
      public:
        FreeListAllocator(size_t buffer_size,
                          Allocator* backing_allocator = MemorySystem::GetDefaultAllocator(),
                          const char* debug_name = "");

        ~FreeListAllocator();

      private:
        Allocator* m_BackingAllocator;
        void* m_BufferStart;
    };
} // namespace Screwjank
