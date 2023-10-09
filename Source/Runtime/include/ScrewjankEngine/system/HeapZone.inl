#include <ScrewjankEngine/system/HeapZone.hpp>

namespace sj
{
    ///////////////////////////////////////////////////////////////////////
    /// Base Heap Zone
    ///////////////////////////////////////////////////////////////////////

	template <class T>
    inline void* HeapZone::AllocateType()
    {
        return Allocate(sizeof(T), alignof(T));
    }

    template <class T, class... Args>
    inline T* HeapZone::New(Args&&... args)
    {
        return new(this->Allocate(sizeof(T), alignof(T))) T(std::forward<Args>(args)...);
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

    template <is_allocator AllocatorType>
    inline THeapZone<AllocatorType>::THeapZone(HeapZone* parent,
                                               const size_t size,
                                               const char* debug_name)
        : m_ParentZone(parent)
    {
        s_HeapZoneList.add(this);

        if(m_ParentZone)
        {
            m_ZoneStart = parent->Allocate(size);
        }
        else
        {
            m_ZoneStart = malloc(size);
        }

        m_ZoneEnd =
            (void*)(reinterpret_cast<uintptr_t>(m_ZoneStart) + size);

        m_Allocator.Init(size, m_ZoneStart);

#ifndef SJ_GOLD
        sj_strncpy(m_DebugName, debug_name, sizeof(m_DebugName));
#endif // !SJ_GOLD
    }

    template <is_allocator AllocatorType>
    inline THeapZone<AllocatorType>::~THeapZone()
    {
        s_HeapZoneList.erase_element(this);

        if(m_ParentZone)
        {
            m_ParentZone->Free(m_ZoneStart);
            m_ZoneStart = nullptr;
        }
        else
        {
            free(m_ZoneStart);
            m_ZoneStart = nullptr;
        }
    }

    template <is_allocator AllocatorType>
    inline void* THeapZone<AllocatorType>::Allocate(const size_t size, const size_t alignment)
    {
        std::scoped_lock<std::mutex> lock(m_HeapLock);
        return m_Allocator.Allocate(size, alignment);
    }
    
    template <is_allocator AllocatorType>
    inline void THeapZone<AllocatorType>::Free(void* memory)
    {
        std::scoped_lock<std::mutex> lock(m_HeapLock);
        m_Allocator.Free(memory);
    }

    template <is_allocator AllocatorType>
    bool THeapZone<AllocatorType>::ContainsPointer(void* ptr) const
    {
        uintptr_t ptr_int = reinterpret_cast<uintptr_t>(ptr);

        return ptr_int >= reinterpret_cast<uintptr_t>(m_ZoneStart) &&
               ptr_int < reinterpret_cast<uintptr_t>(m_ZoneEnd);
    }
}