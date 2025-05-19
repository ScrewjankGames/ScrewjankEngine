#pragma once

// STD Headers
#include <cstddef>
#include <cstdint>

namespace sj
{
    /**
     * Function that returns the first aligned memory address given certain constraints
     * @param align_of The alignment of the memory being allocated
     * @param size The size of the memory being allocated
     * @param buffer_start The start of the memory region in which we are aligning the pointer
     * @param buffer_size How large the buffer we're aligning in is
     */
    void* AlignMemory(size_t align_of, size_t size, void* buffer_start, size_t buffer_size);

    /**
     * Calculates how far the memory is from being aligned
     * @param align_of The alignment requirement of the memory being aligned
     * @return How many bytes ptr is from the last aligned address
     */
    uintptr_t GetAlignmentOffset(size_t align_of, const void* const ptr);

    /**
     * Calculates how many bytes should be added to ptr to get an aligned address
     */
    uintptr_t GetAlignmentAdjustment(size_t align_of, const void* const ptr);

    /**
     * Simple query for whether a memory address satisfies an alignment requirement
     * @param memory_address The memory address to test
     * @param align_of The alignment requirement to test
     * @return Whether memory_address is aligned
     */
    bool IsMemoryAligned(const void* const memory_address, const size_t align_of);

    /**
     * Tests whether or not provided pointer in in the range [space_start, space_end)
     */
    [[nodiscard]] bool IsPointerInAddressSpace(const void* pointer, void* space_start, void* space_end);
}