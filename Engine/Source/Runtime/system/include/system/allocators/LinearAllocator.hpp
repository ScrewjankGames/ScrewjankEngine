#pragma once
// STD Headers

// Library Headers

// Engine Headers
#include "system/Allocator.hpp"

namespace Screwjank {

    class LinearAllocator : public Allocator
    {
      public:
        LinearAllocator(size_t buffer_size, const char* debug_name = "");

        ~LinearAllocator();

        void* Allocate(const size_t size, const size_t alignment = 1) override;

        void Free(void* memory = nullptr) override;

        void Reset();

      private:
        /** Pointer to the start of the allocator's managed memory */
        void* m_BufferStart;

        /** Pointer to the first free byte in the linear allocator */
        void* m_CurrFrameStart;

        /** The number of active allocations in this allocator */
        size_t m_NumActiveAllocations;

        /** The amount of memory free in this allocator */
        size_t m_FreeSpace;

        /** This allocator's capacity in bytes */
        size_t m_Capacity;
    };

} // namespace Screwjank
