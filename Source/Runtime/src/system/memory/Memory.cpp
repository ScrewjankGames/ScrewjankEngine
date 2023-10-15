// STD Headers
#include <memory>
#include <cassert>

// Screwjank Headers
#include <ScrewjankEngine/containers/StaticStack.hpp>
#include <ScrewjankEngine/core/Assert.hpp>
#include <ScrewjankEngine/core/Log.hpp>
#include <ScrewjankEngine/system/memory/Memory.hpp>
#include <ScrewjankEngine/system/memory/allocators/UnmanagedAllocator.hpp>
#include <ScrewjankEngine/system/memory/allocators/FreeListAllocator.hpp>

// Root heap sizes
constexpr uint64_t kRootHeapSize = sj::k1_KiB * 64;
constexpr uint64_t kDebugHeapSize = sj::k1_MiB * 64;

[[nodiscard]] void* operator new(size_t num_bytes) noexcept(false)
{
    return sj::MemorySystem::GetCurrentHeapZone()->Allocate(num_bytes);
}

void operator delete(void* memory) noexcept
{
    if (sj::MemorySystem::GetCurrentHeapZone()->ContainsPointer(memory))
    {
        sj::MemorySystem::GetCurrentHeapZone()->Free(memory);
    }
    else
    {
        // Search for correct heapzone for pointer supplied
        sj::HeapZoneBase* heap_zone = sj::HeapZoneBase::FindHeapZoneForPointer(memory);

        SJ_ASSERT(heap_zone != nullptr,
                  "Failed to find heapzone for pointer! Was heapzone destroyed before the pointer "
                  "was deleted?");

        heap_zone->Free(memory);
    }
}

namespace sj {
    /** Used to track the active heap zone. */
    thread_local StaticStack<HeapZoneBase*, 64> g_HeapZoneStack;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// Memory Manager
    ///////////////////////////////////////////////////////////////////////////////////////////////

    MemorySystem::MemorySystem() 
        : m_RootHeapZone(nullptr, kRootHeapSize, "Root Heap")
#ifndef GOLD_VERSION
          , m_DebugHeapZone(nullptr, kDebugHeapSize, "Debug Heap")
#endif // !GOLD_VERSION
    {

    }

    MemorySystem::~MemorySystem()
    {
    }

    MemorySystem* MemorySystem::Get()
    {
        static MemorySystem memSys;
        return &memSys;
    }

    void MemorySystem::PushHeapZone(HeapZoneBase* heap_zone)
    {
        g_HeapZoneStack.Push(heap_zone);
    }

    void MemorySystem::PopHeapZone()
    {
        g_HeapZoneStack.Pop();
    }

    HeapZoneBase* MemorySystem::GetRootHeapZone()
    {
        return &(Get()->m_RootHeapZone);
        return nullptr;
    }

#ifndef SJ_GOLD
    HeapZoneBase* MemorySystem::GetDebugHeapZone()
    {
        return &(Get()->m_DebugHeapZone);
    }
#endif

    HeapZoneBase* MemorySystem::GetCurrentHeapZone()
    {
        MemorySystem* system = Get();

        if(g_HeapZoneStack.IsEmpty())
        {
            return GetRootHeapZone();
        }
        else
        {
            return g_HeapZoneStack.Top();
        }
    }



    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// Memory Management Utility Functions
    ///////////////////////////////////////////////////////////////////////////////////////////////



} // namespace sj
