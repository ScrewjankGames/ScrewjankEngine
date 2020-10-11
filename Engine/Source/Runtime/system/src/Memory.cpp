// STD Headers
#include <memory>
#include <cassert>

// Library Headers

// Screwjank Headers
#include "system/Memory.hpp"

#include "core/Assert.hpp"
#include "core/Log.hpp"
#include "core/MemorySystem.hpp"

using namespace sj;

[[nodiscard]] void* operator new(size_t num_bytes) noexcept(false)
{
    return MemorySystem::GetUnmanagedAllocator()->Allocate(num_bytes);
}

void operator delete(void* memory) noexcept
{
    MemorySystem::GetUnmanagedAllocator()->Free(memory);
}

namespace sj {

    void* AlignMemory(size_t align_of, size_t size, void* buffer_start, size_t buffer_size)
    {
        SJ_ASSERT(align_of != 0, "Zero is not a valid alignment!");

        // try to carve out _Size bytes on boundary _Bound

        uintptr_t adjustment = GetAlignmentAdjustment(align_of, buffer_start);

        if (buffer_size < adjustment || buffer_size - adjustment < size) {
            SJ_LOG_ERROR("Memory alignment cannot be satisfied in provided space.");
            return nullptr;
        }

        // enough room, update
        buffer_start = (void*)((uintptr_t)buffer_start + adjustment);
        return buffer_start;
    }

    uintptr_t GetAlignmentOffset(size_t align_of, const void* const ptr)
    {
        SJ_ASSERT(align_of != 0, "Zero is not a valid memory alignment requirement.");
        return (uintptr_t)(ptr) & (align_of - 1);
    }

    uintptr_t GetAlignmentAdjustment(size_t align_of, const void* const ptr)
    {
        auto offset = GetAlignmentOffset(align_of, ptr);

        // If the address is already aligned, we don't need any adjustment
        if (offset == 0) {
            return 0;
        }

        return align_of - offset;
    }

    bool IsMemoryAligned(const void* const memory_address, const size_t align_of)
    {
        return GetAlignmentOffset(align_of, memory_address) == 0;
    }

} // namespace sj
