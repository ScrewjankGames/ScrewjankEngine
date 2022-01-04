#pragma once

// STD Headers
#include <Mutex>

// Screwjank Engine Headers
#include <ScrewjankEngine/containers/StaticVector.hpp>
#include <ScrewjankEngine/containers/String.hpp>
#include <ScrewjankEngine/core/Assert.hpp>
#include <ScrewjankEngine/system/Allocator.hpp>

namespace sj
{
    /**
     * Represents a named zone of memory on the heap, owns associated memory or defers to parent HeapZone
     */
    class HeapZone
    {
      public:
        static HeapZone* FindHeapZoneForPointer(void* ptr);
        virtual ~HeapZone() = default;

        [[nodiscard]] 
        virtual void* Allocate(const size_t size, const size_t alignment = alignof(std::max_align_t)) = 0;

        /**
         * Deallocates memory from this heapzone 
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
        static StaticVector<HeapZone*, 64> s_HeapZoneList;

        #ifndef GOLD_VERSION
        char m_DebugName[256]; 
        #endif
    };

    /**
     * Specialized heap zone that accepts an allocator type 
     */
    template<is_allocator AllocatorType>
    class THeapZone : public HeapZone
    {
    public:
        THeapZone(HeapZone* parent, const size_t size, const char* debug_name = "");
        ~THeapZone() override;

        [[nodiscard]] 
        void* Allocate(const size_t size,
                       const size_t alignment = alignof(std::max_align_t)) override;

        void Free(void* memory) override;

        bool ContainsPointer(void* ptr) const override;

    private:
        HeapZone* m_ParentZone = nullptr;
        void* m_ZoneStart = nullptr;
        void* m_ZoneEnd = nullptr;
        AllocatorType m_Allocator;

        // TODO: Actually put together a thread friendly memory model and remove me
        std::mutex m_HeapLock;
    };

    /**
     * Helper class that pushes a heap zone onto the stack on creature
     * and pops it when it goes out of scope
     */
    class HeapZoneScope
    {
      public:
        HeapZoneScope(HeapZone* zone);
        HeapZoneScope(const HeapZoneScope& other) = delete;
        HeapZoneScope(HeapZoneScope&& other);

        HeapZoneScope& operator=(const HeapZoneScope& other) = delete;

        ~HeapZoneScope();

        HeapZone& operator*() { return *m_Heap; }
        HeapZone* operator->() { return m_Heap; }
        const HeapZone& operator*() const { return *m_Heap; }
        const HeapZone* operator->() const { return m_Heap; }

        HeapZone* Get() { return m_Heap; }

      private:
        HeapZone* m_Heap;
    };



} // namespace sj

// Include inlines
#include <ScrewjankEngine/system/HeapZone.inl>