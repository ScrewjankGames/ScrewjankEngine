// STD Headers
#include "ScrewjankEngine/system/memory/MemSpace.hpp"
#include <cassert>

// Screwjank Headers
#include <ScrewjankEngine/containers/StaticStack.hpp>
#include <ScrewjankShared/utils/Assert.hpp>
#include <ScrewjankEngine/utils/Log.hpp>
#include <ScrewjankEngine/system/memory/Memory.hpp>
#include <ScrewjankEngine/system/memory/allocators/FreeListAllocator.hpp>

// Root heap sizes
constexpr uint64_t kRootHeapSize = 5_MiB;
constexpr uint64_t kDebugHeapSize = 64_MiB;

[[nodiscard]] void* operator new(size_t num_bytes) noexcept(false)
{
    if(num_bytes == 0)
        num_bytes++;

    return sj::MemorySystem::GetCurrentMemSpace()->Allocate(num_bytes);
}

void operator delete(void* memory) noexcept
{
    bool found = false;
    if(sj::MemorySystem::GetCurrentMemSpace() == sj::MemorySystem::GetUnmanagedMemSpace())
    {
        // Search for correct MemSpace for pointer supplied
        sj::IMemSpace* mem_space = sj::IMemSpace::FindMemSpaceForPointer(memory);

        if(mem_space)
            mem_space->Free(memory);
        else
            sj::MemorySystem::GetUnmanagedMemSpace()->Free(memory);
    }
    else if (sj::MemorySystem::GetCurrentMemSpace()->ContainsPointer(memory))
    {
        sj::MemorySystem::GetCurrentMemSpace()->Free(memory);
    }
    else
    {
        // Search for correct MemSpace for pointer supplied
        sj::IMemSpace* mem_space = sj::IMemSpace::FindMemSpaceForPointer(memory);

        SJ_ASSERT(mem_space != nullptr,
                  "Failed to find MemSpace for pointer! Was MemSpace destroyed before the pointer "
                  "was deleted?");

        mem_space->Free(memory);
    }
}

namespace sj {
    /** Used to track the active heap zone. */
    thread_local StaticStack<IMemSpace*, 64> g_MemSpaceStack;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// Memory Manager
    ///////////////////////////////////////////////////////////////////////////////////////////////

    MemorySystem::MemorySystem() 
        : m_RootMemSpace(nullptr, kRootHeapSize, "Root Heap")
        , m_UnmanagedMemSpace("Unmanaged Allocations")
#ifndef SJ_GOLD
          , m_DebugMemSpace(nullptr, kDebugHeapSize, "Debug Heap")
#endif // !SJ_GOLD
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

    void MemorySystem::PushMemSpace(IMemSpace* mem_space)
    {
        g_MemSpaceStack.Push(mem_space);
    }

    void MemorySystem::PopMemSpace()
    {
        g_MemSpaceStack.Pop();
    }

    IMemSpace* MemorySystem::GetRootMemSpace()
    {
        return &(Get()->m_RootMemSpace);
    }

    UnmanagedMemSpace* MemorySystem::GetUnmanagedMemSpace()
    {
        return &(Get()->m_UnmanagedMemSpace);        
    }

#ifndef SJ_GOLD
    IMemSpace* MemorySystem::GetDebugMemSpace()
    {
        return &(Get()->m_DebugMemSpace);
    }
#endif

    IMemSpace* MemorySystem::GetCurrentMemSpace()
    {
        MemorySystem* system = Get();

        if(g_MemSpaceStack.IsEmpty())
        {
            return GetUnmanagedMemSpace();
        }
        else
        {
            return g_MemSpaceStack.Top();
        }
    }
} // namespace sj
