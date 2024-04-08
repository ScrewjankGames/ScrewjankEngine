// Screwjank Engine Headers
#include <ScrewjankEngine/system/memory/MemSpace.hpp>

#include <ScrewjankEngine/containers/Vector.hpp>
#include <ScrewjankEngine/system/memory/Memory.hpp>
#include <ScrewjankEngine/containers/String.hpp>

namespace sj
{
    // v This is gonna be a fucking thread safety dumpster fire
    constinit static_vector<IMemSpace*, 64> IMemSpace::s_MemSpaceList;

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

} // namespace sj