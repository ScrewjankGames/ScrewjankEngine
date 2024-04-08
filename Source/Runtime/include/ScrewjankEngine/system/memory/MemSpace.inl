#include <ScrewjankEngine/system/memory/MemSpace.hpp>

namespace sj
{
    ///////////////////////////////////////////////////////////////////////
    /// Base Heap Zone
    ///////////////////////////////////////////////////////////////////////

    template <class T>
    inline T* IMemSpace::AllocateType(const size_t count)
    {
        return static_cast<T*>(Allocate(sizeof(T) * count, alignof(T)));     
    }

    template <class T, class... Args>
    inline T* IMemSpace::New(Args&&... args)
    {
        return new(this->AllocateType<T>()) T(std::forward<Args>(args)...);
    }

    template <class T>
    inline void IMemSpace::Delete(T*& memory)
    {
        SJ_ASSERT(this->ContainsPointer(memory), "Freeing pointer from wrong MemSpace!!");
        memory->~T();
        this->Free(memory);
        memory = nullptr;
    }

    ///////////////////////////////////////////////////////////////////////
    /// THeap Zone
    ///////////////////////////////////////////////////////////////////////

    template <allocator_concept AllocatorType>
    inline MemSpace<AllocatorType>::MemSpace(IMemSpace* parent,
                                               const size_t size,
                                               const char* debug_name)
        : m_ParentZone(parent)
    {
        s_MemSpaceList.push_back(this);

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
    inline MemSpace<AllocatorType>::~MemSpace()
    {
        s_MemSpaceList.erase_element(this);

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
    inline void* MemSpace<AllocatorType>::Allocate(const size_t size, const size_t alignment)
    {
        std::scoped_lock<std::mutex> lock(m_HeapLock);
        return m_Allocator.Allocate(size, alignment);
    }
    
    template <allocator_concept AllocatorType>
    inline void MemSpace<AllocatorType>::Free(void* memory)
    {
        std::scoped_lock<std::mutex> lock(m_HeapLock);
        m_Allocator.Free(memory);
    }

    template <allocator_concept AllocatorType>
    bool MemSpace<AllocatorType>::ContainsPointer(void* ptr) const
    {
        uintptr_t ptr_int = reinterpret_cast<uintptr_t>(ptr);

        return ptr_int >= m_Allocator.Begin() &&
               ptr_int < m_Allocator.End();
    }
}