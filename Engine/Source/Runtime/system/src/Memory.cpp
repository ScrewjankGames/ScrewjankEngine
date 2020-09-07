// STD Headers
#include <memory>
#include <cassert>

// Library Headers

// Screwjank Headers
#include "system/Memory.hpp"

#include "core/Assert.hpp"
#include "core/Log.hpp"
#include "system/allocators/UnmanagedAllocator.hpp"

using namespace Screwjank;

void* operator new(size_t num_bytes) noexcept(false)
{
    return GlobalAllocator()->Allocate(num_bytes);
}

void operator delete(void* memory) throw()
{
    GlobalAllocator()->Free(memory);
}

namespace Screwjank {

    Allocator* GlobalAllocator()
    {
        static Screwjank::UnmanagedAllocator s_Allocator;
        return &s_Allocator;
    }

    void* AlignMemory(size_t align_of, size_t size, void* buffer_start, size_t buffer_size)
    {
        if (align_of == 0) {
            return buffer_start;
        }

        // try to carve out _Size bytes on boundary _Bound
        uintptr_t offset = GetAlignmentOffset(align_of, buffer_start);

        if (offset != 0) {
            offset = align_of - offset; // number of bytes to skip
        }

        if (buffer_size < offset || buffer_size - offset < size) {
            SJ_LOG_ERROR("Memory alignment cannot be satisfied in provided space.");
            return nullptr;
        }

        // enough room, update
        buffer_start = (void*)((uintptr_t)buffer_start + offset);
        return buffer_start;
    }

    uintptr_t GetAlignmentOffset(size_t align_of, const void* const ptr)
    {
        SJ_ASSERT(align_of != 0, "Zero is not a valid memory alignment requirement.");
        return (uintptr_t)(ptr) & (align_of - 1);
    }

} // namespace Screwjank
