#pragma once
// STD Headers
#include <cstddef>
#include <memory>

// Library Headers

// Screwjank Headers
#include "core/Assert.hpp"
#include "system/Allocator.hpp"

namespace sj {

    /**
     * Class that manage's engines default memory allocator and provides access to system memory
     * info
     */
    class MemorySystem
    {
      public:
        /**
         * Provides global access to the memory system
         */
        static MemorySystem* Get();

        /**
         * Provides global access to the engine's default allocator
         * @note This allocator is invalid until the engine calls Initialize()
         */
        static Allocator* GetDefaultAllocator();

        /**
         * Provides global access to an unmanaged system allocator
         * @note This allocator is valid at any time
         */
        static Allocator* GetUnmanagedAllocator();

      private:
        friend class Game;
        /**
         * Initializes memory system and reserves a block of memory for the default allocator
         */
        void Initialize();

      private:
        /////////////////////////////////////////////////////////
        /// Do not access variables and functions in this section
        /////////////////////////////////////////////////////////

        Allocator* m_DefaultAllocator;
        MemorySystem();
        ~MemorySystem();
    };

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
    template <class T, AllocatorConcept Alloc_t, class... Args>
    T* New(Alloc_t& allocator, Args&&... args)
    {
        return new (allocator.AllocateType<T>()) T(std::forward<Args>(args)...);
    }

    /**
     * Global utility function to allocate and construct and object
     * @param allocator Pointer to the allocator to use
     * @param args Arguments to forward to T's constructor
     */
    template <class T, AllocatorPtrConcept Alloc_t, class... Args>
    T* New(Alloc_t& allocator, Args&&... args)
    {
        SJ_ASSERT(allocator != nullptr, "Allocator is invalid.");
        return new (allocator->AllocateType<T>()) T(std::forward<Args>(args)...);
    }

    /**
     * Global utility function to deallocate and destroy and object
     * @param allocator Pointer to the allocator to use
     * @param memory The memory address to free
     */
    template <class T, AllocatorPtrConcept Alloc_t, class... Args>
    void Delete(Alloc_t allocator, T*& memory)
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
    template <class T, AllocatorConcept Alloc_t, class... Args>
    void Delete(Alloc_t& allocator, T*& memory)
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
    using UniquePtr = std::unique_ptr<T, std::function<void(T*)>>;

    template <typename T, AllocatorConcept Alloc_t, typename... Args>
    constexpr UniquePtr<T> MakeUnique(Alloc_t& allocator, Args&&... args)
    {
        // Allocate the memory using the desired allocator
        auto memory = New<T>(allocator, std::forward<Args>(args)...);

        //  Pass ownership of memory to the unique_ptr
        //  Supply a custom deletion function that uses the correct allocator
        return std::unique_ptr<T, std::function<void(T*)>>(memory, [&allocator](T* mem) {
            allocator.Free(mem);
        });
    }

    template <typename T, AllocatorPtrConcept Alloc_t, typename... Args>
    constexpr UniquePtr<T> MakeUnique(Alloc_t allocator, Args&&... args)
    {
        // Allocate the memory using the desired allocator
        auto memory = New<T>(allocator, std::forward<Args>(args)...);

        //  Pass ownership of memory to the unique_ptr
        //  Supply a custom deletion function that uses the correct allocator
        return std::unique_ptr<T, std::function<void(T*)>>(memory, [allocator](T* mem) {
            SJ_ASSERT(allocator != nullptr, "Allocator no longer valid at delete time!");
            allocator->Free(mem);
        });
    }

    template <typename T, typename... Args>
    constexpr UniquePtr<T> MakeUnique(Args&&... args)
    {
        // Leverages global new and delete
        return std::unique_ptr<T>(std::forward<Args>(args)..., [](T* mem) {
            delete mem
        });
    }

    // Placeholder SharedPtr alias
    template <typename T>
    using SharedPtr = std::shared_ptr<T>;

    template <typename T, typename... Args>
    constexpr SharedPtr<T> MakeShared(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

} // namespace sj
