#pragma once

// STD Header

// Library Header

// Screwjank Header
#include "system/Allocator.hpp"

namespace Screwjank {
    /**
     * Class that forwards all of it's allocation logic to it's backing allocator, but stores it's
     * own allocator metrics
     */
    class ProxyAllocator : public Allocator
    {
      public:
        ProxyAllocator(Allocator* backing_allocator, const char* debug_name);
        ~ProxyAllocator();

        [[nodiscard]] void* Allocate(const size_t size, const size_t alignment = 1) override;

        void Free(void* memory) override;

      private:
        Allocator* m_BackingAllocator;
        const char* m_Name;

        /** Structure used to track and report the state of this allocator */
        AllocatorStatus m_AllocatorStats;
    };

} // namespace Screwjank
