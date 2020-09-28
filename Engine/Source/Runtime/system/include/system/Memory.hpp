#pragma once
// STD Headers
#include <cstddef>
#include <memory>

// Library Headers

// Screwjank Headers
#include "core/Assert.hpp"
#include "system/Allocator.hpp"

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

    /**
     * Simple query for whether a memory address satisfies an alignment requirement
     * @param memory_address The memory address to test
     * @param align_of The alignment requirement to test
     * @return Whether memory_address is aligned
     */
    bool IsMemoryAligned(const void* const memory_address, const size_t align_of);

    /**
     * Global utility function to allocate and construct and object using the engine's default
     * allocator
     * @param args Arguments to forward to T's constructor
     * @note This function should not be used until AFTER the engine initialized the allocator.
     */
    template <class T, class... Args>
    T* New(Args&&... args)
    {
        auto allocator = MemorySystem::GetDefaultAllocator();

        SJ_ASSERT(allocator != nullptr, "Engine defualt allocator is not initialized.");

        return New<T>(allocator, std::forward<Args>(args)...);
    }

    /**
     * Global utility function to allocate and construct and object
     * @param allocator The allocator to use
     * @param args Arguments to forward to T's constructor
     */
    template <class T, AllocatorConcept AllocatorType, class... Args>
    T* New(AllocatorType& allocator, Args&&... args)
    {
        return new (allocator.AllocateType<T>()) T(std::forward<Args>(args)...);
    }

    /**
     * Global utility function to allocate and construct and object
     * @param allocator The allocator to use
     * @param args Arguments to forward to T's constructor
     */
    template <class T, AllocatorPtrConcept AllocatorType, class... Args>
    T* New(AllocatorType& allocator, Args&&... args)
    {
        SJ_ASSERT(allocator != nullptr, "Allocator is invalid.");
        return new (allocator->AllocateType<T>()) T(std::forward<Args>(args)...);
    }

    /**
     * Global utility function to deallocate and destroy and object
     * @param allocator Pointer to the allocator to use
     * @param memory The memory address to free
     */
    template <class T, class... Args>
    void Delete(Allocator* allocator, T*& memory)
    {
        // Call object destructor
        memory->~T();

        // Deallocate object
        allocator->Free(memory);

        // Null out supplied pointer
        memory = nullptr;
    }

    /**
     * Global utility function to deallocate and destroy and object
     * @param allocator Pointer to the allocator to use
     * @param memory The memory address to free
     */
    template <class T, class... Args>
    void Delete(Allocator& allocator, T*& memory)
    {
        // Call object destructor
        memory->~T();

        // Deallocate object
        allocator.Free(memory);

        // Null out supplied pointer
        memory = nullptr;
    }

    /**
     * Global utility function to deallocate and destroy and object using the engine's default
     * allocator
     * @param allocator The allocator to use
     * @param memory The memory address to free
     * @note This function should not be used until AFTER the engine initialized the
     * allocator.
     */
    template <class T, class... Args>
    void Delete(T*& memory)
    {
        Delete(MemorySystem::GetDefaultAllocator(), memory);
    }

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
