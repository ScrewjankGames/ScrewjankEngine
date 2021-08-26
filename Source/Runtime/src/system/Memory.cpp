// STD Headers
#include <memory>
#include <cassert>

// Screwjank Headers
#include <ScrewjankEngine/system/Memory.hpp>
#include <ScrewjankEngine/core/Assert.hpp>
#include <ScrewjankEngine/core/Log.hpp>
#include <ScrewjankEngine/system/allocators/UnmanagedAllocator.hpp>
#include <ScrewjankEngine/system/allocators/FreeListAllocator.hpp>

// Root heap sizes
constexpr uint64_t kRootHeapSize = sj::k1_KiB * 64;
constexpr uint64_t kDebugHeapSize = sj::k1_GiB;

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
        sj::HeapZone* heap_zone = sj::HeapZone::FindHeapZoneForPointer(memory);

        SJ_ASSERT(heap_zone != nullptr,
                  "Failed to find heapzone for pointer! Was heapzone destroyed before the pointer "
                  "was deleted?");

        heap_zone->Free(memory);
    }
}

namespace sj {
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

    void MemorySystem::PushHeapZone(HeapZone* heap_zone)
    {
        Get()->m_HeapZoneStack.Push(heap_zone);
    }

    void MemorySystem::PopHeapZone()
    {
        Get()->m_HeapZoneStack.Pop();
    }

    HeapZone* MemorySystem::GetRootHeapZone()
    {
        return &(Get()->m_RootHeapZone);
        return nullptr;
    }

#ifndef SJ_GOLD
    HeapZone* MemorySystem::GetDebugHeapZone()
    {
        return &(Get()->m_DebugHeapZone);
    }
#endif

    HeapZone* MemorySystem::GetCurrentHeapZone()
    {
        MemorySystem* system = Get();

        if (system->m_HeapZoneStack.IsEmpty())
        {
            return GetRootHeapZone();
        }
        else
        {
            return system->m_HeapZoneStack.Top();
        }
    }



    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// Memory Management Utility Functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    void* AlignMemory(size_t align_of, size_t size, void* buffer_start, size_t buffer_size)
    {
        SJ_ASSERT(align_of != 0, "Zero is not a valid alignment!");

        // try to carve out _Size bytes on boundary _Bound

        uintptr_t adjustment = GetAlignmentAdjustment(align_of, buffer_start);

        if (buffer_size < adjustment || buffer_size - adjustment < size) {
            SJ_ENGINE_LOG_ERROR("Memory alignment cannot be satisfied in provided space.");
            return nullptr;
        }

        // enough room, update
        buffer_start = (void*)((uintptr_t)buffer_start + adjustment);
        return buffer_start;
    }

    uintptr_t GetAlignmentOffset(size_t align_of, const void* const ptr)
    {
        SJ_ASSERT(align_of != 0, "Zero is not a valid memory alignment requirement.");
        return (uintptr_t)(ptr) & (align_of - 1);
    }

    uintptr_t GetAlignmentAdjustment(size_t align_of, const void* const ptr)
    {
        auto offset = GetAlignmentOffset(align_of, ptr);

        // If the address is already aligned, we don't need any adjustment
        if (offset == 0) {
            return 0;
        }

        return align_of - offset;
    }

    bool IsMemoryAligned(const void* const memory_address, const size_t align_of)
    {
        return GetAlignmentOffset(align_of, memory_address) == 0;
    }

} // namespace sj
