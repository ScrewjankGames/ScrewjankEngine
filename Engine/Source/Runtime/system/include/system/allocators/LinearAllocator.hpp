#pragma once
// STD Headers

// Library Headers

// Engine Headers
#include "system/Allocator.hpp"
#include "core/MemorySystem.hpp"

namespace Screwjank {

    class LinearAllocator : public Allocator
    {
      public:
        LinearAllocator(size_t buffer_size,
                        Allocator* backing_allocator = MemorySystem::GetDefaultAllocator(),
                        const char* debug_name = "");

        ~LinearAllocator();

        void* Allocate(const size_t size, const size_t alignment = 1) override;

        void Free(void* memory = nullptr) override;

        void Reset();

      private:
        Allocator* m_BackingAllocator;

        /** Pointer to the start of the allocator's managed memory */
        void* m_BufferStart;

        /** Pointer to the first free byte in the linear allocator */
        void* m_CurrFrameStart;
    };

} // namespace Screwjank
