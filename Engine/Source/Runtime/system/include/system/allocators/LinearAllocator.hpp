#pragma once
// STD Headers

// Library Headers

// Engine Headers
#include "core/MemorySystem.hpp"
#include "system/Allocator.hpp"

namespace sj {

    class LinearAllocator : public Allocator
    {
      public:
        LinearAllocator(size_t buffer_size,
                        Allocator* backing_allocator = MemorySystem::GetDefaultAllocator());

        ~LinearAllocator();

        [[nodiscard]] void* Allocate(const size_t size,
                                     const size_t alignment = alignof(std::max_align_t)) override;

        void Free(void* memory = nullptr) override;

        void Reset();

      private:
        Allocator* m_BackingAllocator;

        /** Pointer to the start of the allocator's managed memory */
        void* m_BufferStart;

        /** Pointer to the first free byte in the linear allocator */
        void* m_CurrFrameStart;

        /** Structure used to track and report the state of this allocator */
        AllocatorStatus m_AllocatorStats;
    };

} // namespace sj
