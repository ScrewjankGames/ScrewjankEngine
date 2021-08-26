#pragma once
// Screwjank Engine Headers
//#include <containers/Array.hpp>
#include <core/Assert.hpp>
#include <system/allocators/FreeListAllocator.hpp>

namespace sj
{
    /**
     * Represents a named zone of memory on the heap, owns associated memory or defers to parent HeapZone
     */
    class HeapZone
    {
      public:
        static HeapZone* FindHeapZoneForPointer(void* ptr);

        HeapZone(HeapZone* parent, const size_t size, const char* debug_name = "");
        ~HeapZone();

        [[nodiscard]] void* Allocate(const size_t size,
                                             const size_t alignment = alignof(std::max_align_t));

        /**
         * Deallocates memory from this heapzone 
         */
        void Free(void* memory);

        /**
         * @param args... Arguments to be forwarded to the type's constructor
         * @return Pointer to object of type T in this heap.
         */
        template <class T, class... Args>
        [[nodiscard]] T* New(Args&&... args) { return m_Allocator.New<T>(std::forward<Args>(args)...); }

        /**
         * Helper function that utilizes the allocator to destruct and deallocate an object
         * @param memory Pointer to the object to be deleted
         */
        template <class T>
        void Delete(T*& memory) { 
            SJ_ASSERT(ContainsPointer(memory), "Freeing pointer from wrong HeapZone!!");
            m_Allocator.Delete(memory); 
        }

        bool ContainsPointer(void* ptr) const;

      private:
        HeapZone* m_ParentZone;
        void* m_ZoneStart;
        FreeListAllocator m_Allocator;

        #ifndef GOLD_VERSION
        char m_DebugName[256]; 
        #endif
    };

    /**
     * Helper class that pushes a heap zone onto the stack on creature
     * and pops it when it goes out of scope
     */
    class ScopedHeapZone
    {
      public:
        ScopedHeapZone(HeapZone* zone);
        ScopedHeapZone(const ScopedHeapZone& other) = delete;
        ScopedHeapZone(ScopedHeapZone&& other);

        ScopedHeapZone& operator=(const ScopedHeapZone& other) = delete;

        ~ScopedHeapZone();

        HeapZone& operator*() { return *m_Heap; }
        HeapZone* operator->() { return m_Heap; }
        const HeapZone& operator*() const { return *m_Heap; }
        const HeapZone* operator->() const { return m_Heap; }

        HeapZone* Get() { return m_Heap; }

      private:
        HeapZone* m_Heap;
    };
} // namespace sj