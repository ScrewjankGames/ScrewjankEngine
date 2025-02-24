#pragma once

// STD Headers
#include <memory>

// Screwjank Headers
#include <ScrewjankEngine/system/memory/Allocator.hpp>
#include <ScrewjankEngine/system/memory/MemSpace.hpp>
#include <ScrewjankEngine/system/memory/allocators/FreeListAllocator.hpp>

inline constexpr uint64_t operator""_KiB(unsigned long long val) {return val * 1024;}
inline constexpr uint64_t operator""_MiB(unsigned long long val) {return val * 1024 * 1024;} 
inline constexpr uint64_t operator""_GiB(unsigned long long val) {return val * 1024 * 1024 * 1024;} 

namespace sj 
{
    // Forward declarations
    class IMemSpace;


    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// Memory Manager
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class MemorySystem
    {
      public:
        static void Init();

        /**
         * Provides global access to the memory system
         */
        static MemorySystem* Get();

        static void PushMemSpace(IMemSpace* mem_space);
        static void PopMemSpace();

        static IMemSpace* GetRootMemSpace();

        static UnmanagedMemSpace* GetUnmanagedMemSpace();

#ifndef SJ_GOLD
        /**
         * Used to access debug heap for allocation
         */
        static IMemSpace* GetDebugMemSpace();
#endif // !SJ_GOLD

        /**
         * Users can push and pop heap zones off the stack to control allocations for scopes
         * Allows custom allocators to be used with third party libraries.
         */
        static IMemSpace* GetCurrentMemSpace();

      private:
        UnmanagedMemSpace m_UnmanagedMemSpace;
        #ifndef SJ_GOLD
        MemSpace<FreeListAllocator> m_DebugMemSpace;
        #endif
        MemSpace<FreeListAllocator> m_RootMemSpace;

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
        UniquePtr(T* ptr);

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
        void Reset(T* ptr = nullptr);

        /**
         * Returns underlying raw pointer
         */
        T* Get() const { return m_Pointer; }



      private:
        /** The resource being managed by this container */
        T* m_Pointer;

        /** Deletes managed pointer and resets smart pointer */
        void CleanUp();
    };

    template <typename T, typename... Args>
    constexpr UniquePtr<T> MakeUnique(IMemSpace* zone, Args&&... args);

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
    class UnmanagedMemSpaceScope : public MemSpaceScope
    {
      public:
        UnmanagedMemSpaceScope() 
          : MemSpaceScope(MemorySystem::GetUnmanagedMemSpace())
        {
          MemorySystem::GetUnmanagedMemSpace()->SetLocked(false);
        }

        ~UnmanagedMemSpaceScope()
        {
          MemorySystem::GetUnmanagedMemSpace()->SetLocked(true);
        }
    };

    /**
     * Global utility function to allocate and construct and object from the root heap
     * @param args Arguments to forward to T's constructor
     * @note This function should not be used until AFTER the engine initialized the allocator.
     */
    template <class T, class... Args>
    T* New(Args&&... args)
    {
        IMemSpace* mem_space = MemorySystem::GetCurrentMemSpace();
        return mem_space->New<T>(std::forward<Args>(args)...);
    }

    /**
     * Global utility function to allocate and construct and object from the root heap
     * @param args Arguments to forward to T's constructor
     * @note This function should not be used until AFTER the engine initialized the allocator.
     */
    template <class T, class... Args>
    T* New(IMemSpace* mem_space, Args&&... args)
    {
        return mem_space->New<T>(std::forward<Args>(args)...);
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
    void Delete(IMemSpace* mem_space, T*& memory)
    {
        mem_space->Delete(memory);
    }

} // namespace sj

// Include Inlines
#include <ScrewjankEngine/system/memory/Memory.inl>