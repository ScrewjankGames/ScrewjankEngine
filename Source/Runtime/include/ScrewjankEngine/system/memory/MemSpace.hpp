#pragma once

// STD Headers
#include <mutex>
#include <cstddef>

// Screwjank Engine Headers
#include <ScrewjankEngine/containers/StaticVector.hpp>
#include <ScrewjankShared/utils/Assert.hpp>
#include <ScrewjankEngine/system/memory/Allocator.hpp>
#include <ScrewjankEngine/system/memory/allocators/FreeListAllocator.hpp>

namespace sj
{
    /**
     * Represents a named zone of memory on the heap, owns associated memory or defers to parent MemSpace
     */
    class IMemSpace
    {
      public:
        static IMemSpace* FindMemSpaceForPointer(void* ptr);
        virtual ~IMemSpace() = default;

        [[nodiscard]] 
        virtual void* Allocate(const size_t size, const size_t alignment = alignof(std::max_align_t)) = 0;

        template <class T>
        T* AllocateType(const size_t count = 1);

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

        void Free(void* memory) override;

        bool ContainsPointer(void* ptr) const override;

    private:
        IMemSpace* m_ParentZone = nullptr;
        AllocatorType m_Allocator;

        // TODO: Actually put together a thread friendly memory model and remove me
        std::mutex m_HeapLock;
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