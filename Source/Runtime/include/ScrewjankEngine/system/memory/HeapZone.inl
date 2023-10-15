#include <ScrewjankEngine/system/memory/HeapZone.hpp>

namespace sj
{
    ///////////////////////////////////////////////////////////////////////
    /// Base Heap Zone
    ///////////////////////////////////////////////////////////////////////

    template <class T>
    inline T* HeapZone::AllocateType(const size_t count)
    {
        return static_cast<T*>(Allocate(sizeof(T) * count, alignof(T)));     
    }

    template <class T, class... Args>
    inline T* HeapZone::New(Args&&... args)
    {
        return new(this->AllocateType<T>()) T(std::forward<Args>(args)...);
    }

    template <class T>
    inline void HeapZone::Delete(T*& memory)
    {
        SJ_ASSERT(this->ContainsPointer(memory), "Freeing pointer from wrong HeapZone!!");
        memory->~T();
        this->Free(memory);
        memory = nullptr;
    }

    ///////////////////////////////////////////////////////////////////////
    /// THeap Zone
    ///////////////////////////////////////////////////////////////////////

    template <allocator_concept AllocatorType>
    inline THeapZone<AllocatorType>::THeapZone(HeapZone* parent,
                                               const size_t size,
                                               const char* debug_name)
        : m_ParentZone(parent)
    {
        s_HeapZoneList.add(this);

        void* start;

        if(m_ParentZone)
        {
            start = parent->Allocate(size);
        }
        else
        {
            start = malloc(size);
        }

        m_Allocator.Init(size, start);

#ifndef SJ_GOLD
        sj_strncpy(m_DebugName, debug_name, sizeof(m_DebugName));
#endif // !SJ_GOLD
    }

    template <allocator_concept AllocatorType>
    inline THeapZone<AllocatorType>::~THeapZone()
    {
        s_HeapZoneList.erase_element(this);

        if(m_ParentZone)
        {
            m_ParentZone->Free(reinterpret_cast<void*>(m_Allocator.Begin()));
        }
        else
        {
            free(reinterpret_cast<void*>(m_Allocator.Begin()));
        }
    }

    template <allocator_concept AllocatorType>
    inline void* THeapZone<AllocatorType>::Allocate(const size_t size, const size_t alignment)
    {
        std::scoped_lock<std::mutex> lock(m_HeapLock);
        return m_Allocator.Allocate(size, alignment);
    }
    
    template <allocator_concept AllocatorType>
    inline void THeapZone<AllocatorType>::Free(void* memory)
    {
        std::scoped_lock<std::mutex> lock(m_HeapLock);
        m_Allocator.Free(memory);
    }

    template <allocator_concept AllocatorType>
    bool THeapZone<AllocatorType>::ContainsPointer(void* ptr) const
    {
        uintptr_t ptr_int = reinterpret_cast<uintptr_t>(ptr);

        return ptr_int >= m_Allocator.Begin() &&
               ptr_int < m_Allocator.End();
    }
}