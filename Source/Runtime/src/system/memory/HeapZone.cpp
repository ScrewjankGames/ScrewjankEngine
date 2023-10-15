// Screwjank Engine Headers
#include <ScrewjankEngine/system/memory/HeapZone.hpp>

#include <ScrewjankEngine/containers/Vector.hpp>
#include <ScrewjankEngine/system/memory/Memory.hpp>
#include <ScrewjankEngine/containers/String.hpp>

namespace sj
{
    // v This is gonna be a fucking thread safety dumpster fire
    constinit static_vector<HeapZone*, 64> HeapZone::s_HeapZoneList;

    HeapZone* HeapZone::FindHeapZoneForPointer(void* ptr)
    {
        for(HeapZone* zone : HeapZone::s_HeapZoneList)
        {
            if(zone && zone->ContainsPointer(ptr))
            {
                return zone;
            }
        }

        return nullptr;
    }

    HeapZoneScope::HeapZoneScope(HeapZone* zone) : m_Heap(zone)
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