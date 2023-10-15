// Screwjank Engine Headers
#include <ScrewjankEngine/system/memory/HeapZone.hpp>

#include <ScrewjankEngine/containers/Vector.hpp>
#include <ScrewjankEngine/system/memory/Memory.hpp>
#include <ScrewjankEngine/containers/String.hpp>

namespace sj
{
    // v This is gonna be a fucking thread safety dumpster fire
    constinit static_vector<HeapZoneBase*, 64> HeapZoneBase::s_HeapZoneList;

    HeapZoneBase* HeapZoneBase::FindHeapZoneForPointer(void* ptr)
    {
        for(HeapZoneBase* zone : HeapZoneBase::s_HeapZoneList)
        {
            if(zone && zone->ContainsPointer(ptr))
            {
                return zone;
            }
        }

        return nullptr;
    }

    HeapZoneScope::HeapZoneScope(HeapZoneBase* zone) : m_Heap(zone)
    {
        MemorySystem::PushHeapZone(zone);
    }

    HeapZoneScope::HeapZoneScope(HeapZoneScope&& other) : HeapZoneScope(other.m_Heap)
    {
        other.m_Heap = nullptr;
    }

    HeapZoneScope::~HeapZoneScope()
    {
        MemorySystem::PopHeapZone();
    }

} // namespace sj