#pragma once

// STD Header

// Library Header

// Screwjank Header
#include "system/Allocator.hpp"

namespace sj {
    /**
     * Class that forwards all of it's allocation logic to it's backing allocator, but stores it's
     * own allocator metrics
     */
    class ProxyAllocator : public Allocator
    {
      public:
        /**
         * Constructor
         * @param backing_allocator The allocator instance this proxy serves as an adapter to
         * @param debug_name A debug name used to identify the proxy instance
         */
        ProxyAllocator(Allocator* backing_allocator, const char* debug_name);

        /**
         * Destructor
         */
        ~ProxyAllocator();

        /**
         * Forwards allocation call to m_BackingAllocator
         */
        [[nodiscard]] void* Allocate(const size_t size,
                                     const size_t alignment = alignof(std::max_align_t)) override;

        /**
         * Forwards free call to m_BackingAllocator
         */
        void Free(void* memory) override;

      private:
        /** The allocator this proxy uses to perform allocations */
        Allocator* m_BackingAllocator;

        /** Debug name of the allocator */
        const char* m_Name;

        /** Structure used to track and report the state of this allocator */
        AllocatorStatus m_AllocatorStats;
    };

} // namespace sj
