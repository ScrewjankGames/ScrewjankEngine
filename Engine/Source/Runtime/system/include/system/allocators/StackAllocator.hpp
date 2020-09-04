#pragma once
// STD Headers
#include <cstddef>

// Library Headers

// Engine Headers
#include "system/Allocator.hpp"

namespace Screwjank {

    struct StackAllocatorHeader
    {
        std::byte* PreviousHeader;
    };

    class StackAllocator : public Allocator
    {
      public:
        StackAllocator(size_t buffer_size);

        ~StackAllocator();

        void* Allocate(const size_t size, const size_t alignment) override;

        void Free(void* memory = nullptr) override;

        template <typename T>
        void* Push();

        void Pop();

        void Reset();

      private:
        /** Buffer the allocator manages */
        void* m_Buffer;

        size_t m_Capacity;

        size_t m_Used;

        /** Pointer to the start of the current stack frame */
        StackAllocatorHeader* m_CurrFrameHeader;
    };

    template <typename T>
    inline void* StackAllocator::Push()
    {
        return Allocate(sizeof(T), alignof(T));
    }
} // namespace Screwjank
