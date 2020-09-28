#pragma once
// STD Headers

// Library Headers

// Screwjank Headers
#include "system/Allocator.hpp"

namespace sj {
    /**
     * @class UnmanagedAllocator
     * @brief A simple wrapper for standard malloc and free calls
     */
    class UnmanagedAllocator : public Allocator
    {
      public:
        /** Constructor */
        UnmanagedAllocator();

        /** Destructor */
        ~UnmanagedAllocator();

        /** Allocate system memory from the heap */
        [[nodiscard]] void* Allocate(const size_t size, const size_t alignment = 0) override;

        /** Free system memory from the heap */
        void Free(void* memory) override;

      private:
        size_t m_ActiveAllocationCount;
    };
} // namespace sj
