module;

#include <ScrewjankStd/Assert.hpp>

// STD Headers
#include <cstddef>
#include <cstdint>

export module sj.std.memory.utils;

export namespace sj
{
constexpr uintptr_t get_alignment_offset(size_t align_of, const uintptr_t addr)
{
    return addr & (align_of - 1);
}

constexpr uintptr_t get_alignment_adjustment(size_t align_of, const uintptr_t addr)
{
    auto offset = get_alignment_offset(align_of, addr);

    // If the address is already aligned, we don't need any adjustment
    if(offset == 0)
        return 0;

    return align_of - offset;
}

uintptr_t get_alignment_offset(size_t align_of, const void* const ptr)
{
    SJ_ASSERT(align_of != 0, "Zero is not a valid memory alignment requirement.");
    return get_alignment_offset(align_of, reinterpret_cast<uintptr_t>(ptr));
}

uintptr_t get_alignment_adjustment(size_t align_of, const void* const ptr)
{
    return get_alignment_adjustment(align_of, reinterpret_cast<uintptr_t>(ptr));
}

void* align_memory(size_t align_of, size_t size, void* buffer_start, size_t buffer_size)
{
    SJ_ASSERT(align_of != 0, "Zero is not a valid alignment!");

    // try to carve out _Size bytes on boundary _Bound

    uintptr_t adjustment = get_alignment_adjustment(align_of, buffer_start);

    if(buffer_size < adjustment || buffer_size - adjustment < size)
    {
        SJ_ASSERT(false, "Memory alignment cannot be satisfied in provided space.");
        return nullptr;
    }

    // enough room, update
    buffer_start = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(buffer_start) + adjustment);
    return buffer_start;
}

bool is_memory_aligned(const void* const memory_address, const size_t align_of)
{
    return get_alignment_offset(align_of, memory_address) == 0;
}

[[nodiscard]] bool
is_pointer_in_address_space(const void* pointer, void* space_start, void* space_end)
{
    return uintptr_t(pointer)
           >= uintptr_t(space_start)
           && uintptr_t(pointer)
           < uintptr_t(space_end);
}

} // namespace sj