#pragma once
// STD Headers
#include <cstddef>
#include <memory>
#include <functional>

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

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// Memory Management Utility Functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

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

        return allocator->New<T>(std::forward<Args>(args)...);
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
        MemorySystem::GetDefaultAllocator()->Delete(memory);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// Smart Pointer Implementations
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // Placeholder UniquePtr alias
    // template <typename T>
    // using UniquePtr = std::unique_ptr<T, std::function<void(T*)>>;

    template <typename T>
    class UniquePtr
    {
        using Deleter = std::function<void(T*)>;

      public:
        /**
         * Constructor
         * @param p The pointer to be managed
         * @Note Since a deleter is not supplied, it is assumed the Default Allocator deleter can be
         * used
         */
        UniquePtr(T* ptr = nullptr) : m_Pointer(nullptr)
        {
            m_Deleter = std::move([](T* ptr) {
                MemorySystem::GetDefaultAllocator()->Delete(ptr);
            });
        }

        /**
         * Constructor
         * @param p The pointer to be managed
         * @param deleter The deleter to be used when releasing the pointer
         */
        UniquePtr(T* ptr, Deleter deleter) : m_Pointer(ptr), m_Deleter(std::move(deleter))
        {
        }

        /**
         * Move constructor
         */
        UniquePtr(UniquePtr&& other)
        {
            m_Pointer = other.m_Pointer;
            other.m_Pointer = nullptr;
            m_Deleter = std::move(other.m_Deleter);
        }

        /**
         * Destructor
         */
        ~UniquePtr()
        {
            CleanUp();
        }

        /**
         * Move Assignment Operator
         */
        void operator=(UniquePtr&& other)
        {
            CleanUp();

            m_Pointer = other.m_Pointer;
            other.m_Pointer = nullptr;
            m_Deleter = std::move(other.m_Deleter);
        }

        /**
         * Arrow operator overload
         */
        T* operator->()
        {
            return m_Pointer;
        }

        /**
         * Dereference operator overload
         */
        T& operator*()
        {
            return *m_Pointer;
        }

        /**
         * Releases ownership of managed resource to caller
         * @return A copy of m_Pointer
         */
        [[nodiscard]] T* Release() noexcept
        {
            auto ptr = m_Pointer;
            m_Pointer = nullptr;
            return ptr;
        }

        /**
         * Releases currently managed resource (if present), and asumes ownership of ptr
         * @param ptr The pointer to manage
         * @note Assumes current deleter is sufficient (same allocator as old m_Pointer)
         */
        void Reset(T* ptr = nullptr)
        {
            CleanUp();
            m_Pointer = ptr;
        }

        /**
         * Returns underlying raw pointer
         */
        T* Get() const
        {
            return m_Pointer;
        }

        /**
         * Copy constructor: disallowed
         */
        UniquePtr(const UniquePtr& other) = delete;

        /**
         * Copy Assignment Operator: disallowed
         */
        UniquePtr& operator=(const UniquePtr& other) = delete;

      private:
        /** The resource being managed by this container */
        T* m_Pointer;

        /** Function capable of deleting the contained pointer */
        std::function<void(T*)> m_Deleter;

        void CleanUp()
        {
            if (m_Pointer != nullptr) {
                // Delete the resource with the supplied deleter
                m_Deleter(m_Pointer);
            }
        }
    };

    template <typename T, AllocatorConcept Alloc_t, typename... Args>
    constexpr UniquePtr<T> MakeUnique(Alloc_t& allocator, Args&&... args)
    {
        // Allocate the memory using the desired allocator
        auto memory = allocator.New<T>(std::forward<Args>(args)...);

        //  Pass ownership of memory to the unique_ptr
        //  Supply a custom deletion function that uses the correct allocator
        return std::unique_ptr<T, std::function<void(T*)>>(memory, [&allocator](T* mem) {
            allocator.Delete<T>(mem);
        });
    }

    template <typename T, AllocatorPtrConcept Alloc_t, typename... Args>
    constexpr UniquePtr<T> MakeUnique(Alloc_t allocator, Args&&... args)
    {
        // Allocate the memory using the desired allocator
        auto memory = allocator->New<T>(std::forward<Args>(args)...);

        //  Pass ownership of memory to the unique_ptr
        //  Supply a custom deletion function that uses the correct allocator
        return UniquePtr<T>(memory, [allocator](T* mem) {
            SJ_ASSERT(allocator != nullptr, "Allocator no longer valid at delete time!");
            allocator->Delete<T>(mem);
        });
    }

    template <typename T, typename... Args>
    constexpr UniquePtr<T> MakeUnique(Args&&... args)
    {
        // Get pointer to the engine's default allocator
        auto allocator = MemorySystem::GetDefaultAllocator();

        return MakeUnique<T>(allocator, std::forward<Args>(args)...);
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
