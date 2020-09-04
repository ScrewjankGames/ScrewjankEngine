#pragma once
// STD Headers
#include <cstddef>

// Library Headers

// Screwjank Headers
#include "system/Allocator.hpp"

namespace Screwjank {

    Allocator* GlobalAllocator();

    /**
     * Function that returns an aligned memory address for an object given it's alignment, size, and
     * potential storage area
     * @param align_of The alignment of the memory being allocated
     * @param size The size of the memory being allocated
     * @param buffer_start The start of the memory region in which we are aligning the pointer
     * @param buffer_size How large the buffer we're aligning in is
     */
    void* AlignMemory(size_t align_of, size_t size, void* buffer_start, size_t buffer_size);
} // namespace Screwjank
