#pragma once

// STD Headers
#include <memory_resource>
#include <mutex>
#include <cstddef>

// Screwjank Engine Headers
#include <ScrewjankShared/utils/Assert.hpp>
#include <ScrewjankEngine/system/memory/Allocator.hpp>
#include <ScrewjankEngine/system/memory/allocators/FreeListAllocator.hpp>

import sj.shared.containers;

namespace sj
{
    /**
     * Represents a named zone of memory on the heap, owns associated memory or defers to parent MemSpace
     */
    class IMemSpace : public std::pmr::memory_resource
    {
      public:
        static IMemSpace* FindMemSpaceForPointer(void* ptr);
        virtual ~IMemSpace() = default;

        [[nodiscard]] 
        virtual void* Allocate(const size_t size, const size_t alignment = alignof(std::max_align_t)) = 0;

        template <class T>
        T* AllocateType(const size_t count = 1);

        [[nodiscard]] 
        virtual void* Reallocate(void* originalPtr, const size_t size, const size_t alignment) = 0;

        /**
         * Deallocates memory from this MemSpace 
         */
        virtual void Free(void* memory) = 0;

        /**
         * @param args... Arguments to be forwarded to the type's constructor
         * @return Pointer to object of type T in this heap.
         */
        template <class T, class... Args>
        [[nodiscard]] 
        T* New(Args&&... args);

        /**
         * Helper function that utilizes the allocator to destruct and deallocate an object
         * @param memory Pointer to the object to be deleted
         */
        template <class T>
        void Delete(T*& memory);

        virtual bool ContainsPointer(void* ptr) const = 0;

      protected:
        static static_vector<IMemSpace*, 64> s_MemSpaceList;

        #ifndef GOLD_VERSION
        char m_DebugName[256]; 
        #endif
    };

    /**
     * Specialized heap zone that accepts an allocator type 
     */
    template<allocator_concept AllocatorType = FreeListAllocator>
    class MemSpace final : public IMemSpace
    {
    public:
        MemSpace() = default;
        MemSpace(IMemSpace* parent, const size_t size, const char* debug_name = "");
        ~MemSpace() override;

        void Init(IMemSpace* parent, const size_t size, const char* debug_name = "");

        [[nodiscard]] 
        void* Allocate(const size_t size,
                       const size_t alignment = alignof(std::max_align_t)) override;

        [[nodiscard]] 
        void* Reallocate(void* originalPtr, const size_t size, const size_t alignment) override;

        void Free(void* memory) override;

        bool ContainsPointer(void* ptr) const override;

        void* do_allocate(size_t bytes, size_t alignment) override
        {
          return Allocate(bytes, alignment);
        };
    
        void do_deallocate(void* memory, size_t bytes, size_t alignment) override
        {
          Free(memory);
        };
    
        bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override
        {
          return this == &other;
        }

    private:
        IMemSpace* m_ParentZone = nullptr;
        AllocatorType m_Allocator;

        // TODO: Actually put together a thread friendly memory model and remove me
        std::mutex m_HeapLock;
    };


    /**
     * Memspace for allocations we can't be bothered to track right now
     */
    class UnmanagedMemSpace : public IMemSpace
    {
    public:
        UnmanagedMemSpace(const char* name);
        ~UnmanagedMemSpace() = default;

        [[nodiscard]] 
        void* Allocate(const size_t size,
                       const size_t alignment = alignof(std::max_align_t)) override;

        [[nodiscard]] 
        void* Reallocate(void* originalPtr, const size_t size, const size_t alignment) override;

        void Free(void* memory) override;

        bool ContainsPointer(void* ptr) const override;

        void SetLocked(bool isLocked) { m_isLocked = isLocked; }

        void* do_allocate(size_t bytes, size_t alignment) override
        {
          return Allocate(bytes, alignment);
        };
    
        void do_deallocate(void* memory, size_t bytes, size_t alignment) override
        {
          Free(memory);
        };
    
        bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override
        {
          return this == &other;
        }

    private:
        bool m_isLocked = false;
    };

    /**
     * Helper class that pushes a heap zone onto the stack on creature
     * and pops it when it goes out of scope
     */
    class MemSpaceScope
    {
      public:
        MemSpaceScope(IMemSpace* zone);
        MemSpaceScope(const MemSpaceScope& other) = delete;
        MemSpaceScope(MemSpaceScope&& other);

        MemSpaceScope& operator=(const MemSpaceScope& other) = delete;

        ~MemSpaceScope();

        IMemSpace& operator*() { return *m_Heap; }
        IMemSpace* operator->() { return m_Heap; }
        const IMemSpace& operator*() const { return *m_Heap; }
        const IMemSpace* operator->() const { return m_Heap; }

        IMemSpace* Get() { return m_Heap; }

      private:
        IMemSpace* m_Heap;
    };

    MemSpace() -> MemSpace<FreeListAllocator>; 

} // namespace sj

// Include inlines
#include <ScrewjankEngine/system/memory/MemSpace.inl>