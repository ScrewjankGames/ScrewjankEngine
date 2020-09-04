#pragma once
// STD Headers

// Library Headers

// Engine Headers
#include "system/Allocator.hpp"

namespace Screwjank {

    class LinearAllocator : public Allocator
    {
      public:
        LinearAllocator(size_t buffer_size);

        ~LinearAllocator();

        void* Allocate(const size_t size, const size_t alignment = 0) override;

        void Free(void* memory = nullptr) override;

        void Reset();

        template <class T>
        void* Allocate();

      private:
        /** Pointer to the start of the allocator's managed memory */
        void* m_Buffer;

        /** Pointer to the first free byte in the linear allocator */
        void* m_CurrFrameStart;

        /** This allocator's capacity in bytes */
        size_t m_Capacity;
    };

    template <class T>
    void* LinearAllocator::Allocate()
    {
        return Allocate(sizeof(T), alignof(T));
    }
} // namespace Screwjank
