// Screwjank Engine Headers
#include <ScrewjankEngine/system/memory/MemSpace.hpp>
#include <ScrewjankEngine/system/memory/Memory.hpp>
#include <ScrewjankEngine/containers/String.hpp>

#include <ScrewjankShared/utils/Assert.hpp>

import sj.shared.containers;

namespace sj
{
    // v This is gonna be a fucking thread safety dumpster fire
    static_vector<IMemSpace*, 64> IMemSpace::s_MemSpaceList;

    IMemSpace* IMemSpace::FindMemSpaceForPointer(void* ptr)
    {
        for(IMemSpace* zone : IMemSpace::s_MemSpaceList)
        {
            if(zone && zone->ContainsPointer(ptr))
            {
                return zone;
            }
        }

        return nullptr;
    }

    MemSpaceScope::MemSpaceScope(IMemSpace* zone) : m_Heap(zone)
    {
        MemorySystem::PushMemSpace(zone);
    }

    MemSpaceScope::MemSpaceScope(MemSpaceScope&& other) : MemSpaceScope(other.m_Heap)
    {
        other.m_Heap = nullptr;
    }

    MemSpaceScope::~MemSpaceScope()
    {
        MemorySystem::PopMemSpace();
    }

    UnmanagedMemSpace::UnmanagedMemSpace(const char* name)
    {
#ifndef SJ_GOLD
        strcpy(m_DebugName, name);
#endif
    }

    void* UnmanagedMemSpace::Allocate(const size_t size, const size_t alignment)
    {
        SJ_ASSERT(!m_isLocked, "Unmanaged allocations are prohibited at this time");
        return std::malloc(size);
    }
    
    void* UnmanagedMemSpace::Reallocate(void* originalPtr, const size_t size, const size_t alignment)
    {
        SJ_ASSERT(!m_isLocked, "Unmanaged allocations are prohibited at this time");
        return std::realloc(originalPtr, size);
    }

    void UnmanagedMemSpace::Free(void* memory)
    {
        std::free(memory);
    }

    bool UnmanagedMemSpace::ContainsPointer(void* ptr) const
    {
        SJ_ASSERT(false, "Unsupported operation!");
        return false;
    }

} // namespace sj