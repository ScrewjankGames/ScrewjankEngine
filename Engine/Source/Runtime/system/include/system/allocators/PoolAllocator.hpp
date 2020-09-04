#pragma once
// STD Headers

// Library Headers

// Engine Headers
#include "system/Allocator.hpp"

namespace Screwjank {

    template <size_t block_size>
    class PoolAllocator : public Allocator
    {
      public:
        PoolAllocator(size_t buffer_size);
    };

    template <class T>
    class ObjectPoolAllocator : public PoolAllocator<sizeof(T)>
    {
    };

    template <size_t block_size>
    inline PoolAllocator<block_size>::PoolAllocator(size_t buffer_size)
    {
        static_assert(buffer_size > block_size);
    }

} // namespace Screwjank
