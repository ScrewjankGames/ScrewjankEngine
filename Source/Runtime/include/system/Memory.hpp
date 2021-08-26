#pragma once
// STD Headers
#include <cstddef>
#include <memory>
#include <functional>

// Screwjank Headers
#include <containers/Stack.hpp>
#include <system/Allocator.hpp>
#include <system/HeapZone.hpp>

namespace sj {

    // Forward declarations
    class HeapZone;

    // Common memory sizes
    constexpr uint64_t k1_KiB = 1024;
    constexpr uint64_t k1_MiB = k1_KiB * 1024;
    constexpr uint64_t k1_GiB = k1_MiB * 1024;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// Memory Manager
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class MemorySystem
    {
      public:
        /**
         * Provides global access to the memory system
         */
        static MemorySystem* Get();

        static void PushHeapZone(HeapZone* heap_zone);
        static void PopHeapZone();

        static HeapZone* GetRootHeapZone();

#ifndef SJ_GOLD
        /**
         * Used to access debug heap for allocation
         */
        static HeapZone* GetDebugHeapZone();
#endif // !SJ_GOLD

        /**
         * Users can push and pop heap zones off the stack to control allocations for scopes
         * Allows custom allocators to be used with third party libraries.
         */
        static HeapZone* GetCurrentHeapZone();

      private:
        HeapZone m_RootHeapZone;
        HeapZone m_DebugHeapZone;
        
        /** Used to track the active heap zone. */
        StaticStack<HeapZone*, 64> m_HeapZoneStack;

        MemorySystem();
        ~MemorySystem();
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// Smart Pointers
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    class UniquePtr
    {
      public:
        /** Constructor */
        UniquePtr();

        /**
         * Constructor
         * @param heap Heap the pointer comes from
         * @param p The pointer to be managed
         */
        UniquePtr(HeapZone* heap, T* ptr);

        /**
         * Copy constructor: disallowed
         */
        UniquePtr(const UniquePtr& other) = delete;

        /**
         * Move constructor
         */
        UniquePtr(UniquePtr&& other);

        /** Destructor */
        ~UniquePtr();

        /**
         * Copy Assignment Operator: disallowed
         */
        UniquePtr& operator=(const UniquePtr& other) = delete;

        /**
         * Move Assignment Operator
         */
        void operator=(UniquePtr&& other) noexcept;

        /**
         * Arrow operator overload
         */
        T* operator->() { return m_Pointer; }

        /**
         * Const arrow operator overload
         */
        const T* operator->() const { return m_Pointer; }

        /**
         * Dereference operator overload
         */
        T& operator*() { return *m_Pointer; }

        /**
         * Releases ownership of managed resource to caller
         * @return A copy of m_Pointer
         */
        [[nodiscard]] T* Release() noexcept;

        /**
         * Releases currently managed resource (if present), and asumes ownership of ptr
         * @param ptr The pointer to manage
         * @note Assumes current deleter is sufficient (same allocator as old m_Pointer)
         */
        void Reset(HeapZone* heap = nullptr, T* ptr = nullptr);

        /**
         * Returns underlying raw pointer
         */
        T* Get() const { return m_Pointer; }



      private:
        /** The heap zone the pointer was allocated from */
        HeapZone* m_HeapZone;

        /** The resource being managed by this container */
        T* m_Pointer;

        /** Deletes managed pointer and resets smart pointer */
        void CleanUp();
    };

    template <typename T, typename... Args>
    constexpr UniquePtr<T> MakeUnique(HeapZone* zone, Args&&... args);

    // Placeholder SharedPtr alias
    template <typename T>
    using SharedPtr = std::shared_ptr<T>;

    template <typename T, typename... Args>
    constexpr SharedPtr<T> MakeShared(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// Memory Management Utility Functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * Global utility function to allocate and construct and object from the root heap
     * @param args Arguments to forward to T's constructor
     * @note This function should not be used until AFTER the engine initialized the allocator.
     */
    template <class T, class... Args>
    T* New(Args&&... args)
    {
        HeapZone* heap_zone = MemorySystem::GetCurrentHeapZone();
        return heap_zone->New<T>(std::forward<Args>(args)...);
    }

    /**
     * Global utility function to allocate and construct and object from the root heap
     * @param args Arguments to forward to T's constructor
     * @note This function should not be used until AFTER the engine initialized the allocator.
     */
    template <class T, class... Args>
    T* New(HeapZone* heap_zone, Args&&... args)
    {
        return heap_zone->New<T>(std::forward<Args>(args)...);
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
        memory->~T();
        delete memory;
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
    void Delete(HeapZone* heap_zone, T*& memory)
    {
        heap_zone->Delete(memory);
    }

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

} // namespace sj

// Include Inlines
#include <system/Memory.inl>