#pragma once
// STD Headers
#include <cstddef>
#include <memory>

// Library Headers

// Screwjank Headers

namespace Screwjank {
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

    bool IsMemoryAligned(const void* const memory_address, const size_t align_of);

    // Placeholder UniquePtr alias
    template <typename T>
    using UniquePtr = std::unique_ptr<T>;

    template <typename T, typename... Args>
    constexpr UniquePtr<T> MakeUnique(Args&&... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    // Placeholder SharedPtr alias
    template <typename T>
    using SharedPtr = std::shared_ptr<T>;

    template <typename T, typename... Args>
    constexpr SharedPtr<T> MakeShared(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
} // namespace Screwjank
