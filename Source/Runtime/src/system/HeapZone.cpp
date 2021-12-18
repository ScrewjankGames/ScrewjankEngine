// Screwjank Engine Headers
#include <ScrewjankEngine/system/HeapZone.hpp>

#include <ScrewjankEngine/containers/Vector.hpp>
#include <ScrewjankEngine/system/Memory.hpp>
#include <ScrewjankEngine/containers/String.hpp>

namespace sj
{
    // v This is gonna be a fucking thread safety dumpster fire
    StaticVector<HeapZone*, 64> g_HeapZoneList;
    int g_NumHeapZones = 0;

    HeapZone* HeapZone::FindHeapZoneForPointer(void* ptr)
    {
        for(HeapZone* zone : g_HeapZoneList)
        {
            if(zone && zone->ContainsPointer(ptr))
            {
                return zone;
            }
        }

        return nullptr;
    }

    HeapZone::HeapZone(HeapZone* parent, const size_t size, const char* debug_name)
        : m_ParentZone(parent)
    {
        g_HeapZoneList.Add(this);

        if(m_ParentZone)
        {
            m_ZoneStart = parent->Allocate(size);
        }
        else
        {
            m_ZoneStart = malloc(size);
        }

        m_Allocator.Init(size, m_ZoneStart);

        #ifndef SJ_GOLD
        sj_strncpy(m_DebugName, debug_name, sizeof(m_DebugName));
        #endif // !SJ_GOLD
    }

    HeapZone::~HeapZone()
    {
        g_HeapZoneList.EraseElement(this);

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
    
    void* HeapZone::Allocate(const size_t size, const size_t alignment)
    {
        std::lock_guard<std::mutex> allocGuard(m_AllocMutex);
        return m_Allocator.Allocate(size, alignment);
    }
    
    void HeapZone::Free(void* memory)
    {
        std::lock_guard<std::mutex> allocGuard(m_AllocMutex);
        m_Allocator.Free(memory);
    }

    bool HeapZone::ContainsPointer(void* ptr) const
    {
        return m_Allocator.IsMemoryInRange(ptr);
    }


    ScopedHeapZone::ScopedHeapZone(HeapZone* zone) : m_Heap(zone)
    {
        MemorySystem::PushHeapZone(zone);
    }

    ScopedHeapZone::ScopedHeapZone(ScopedHeapZone&& other) : ScopedHeapZone(other.m_Heap)
    {
        other.m_Heap = nullptr;
    }

    ScopedHeapZone::~ScopedHeapZone()
    {
        MemorySystem::PopHeapZone();
    }

} // namespace sj