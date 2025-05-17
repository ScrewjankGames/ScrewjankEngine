// Parent Include
#include <ScrewjankEngine/system/memory/Memory.hpp>

// STD Headers
#include <cassert>

// Screwjank Headers
#include <ScrewjankShared/utils/Assert.hpp>
#include <ScrewjankShared/utils/Log.hpp>
#include <ScrewjankEngine/system/memory/MemSpace.hpp>
#include <ScrewjankEngine/system/memory/allocators/FreeListAllocator.hpp>

import sj.shared.containers;

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
    sj::IMemSpace* currMemSpace = sj::MemorySystem::GetCurrentMemSpace();
    sj::IMemSpace* unmanagedMemSpace = sj::MemorySystem::GetUnmanagedMemSpace();

    if (currMemSpace && currMemSpace != unmanagedMemSpace && currMemSpace->ContainsPointer(memory))
    {
        currMemSpace->Free(memory);
    }
    else if(sj::MemorySystem::GetRootMemSpace()->ContainsPointer(memory))
    {
        // Search for correct MemSpace for pointer supplied
        sj::IMemSpace* mem_space = sj::IMemSpace::FindMemSpaceForPointer(memory);
        SJ_ASSERT(mem_space != nullptr, "Failed to find managed memory region");
        mem_space->Free(memory);
    }
#ifndef GOLD_VERSION
    else if(sj::MemorySystem::GetDebugMemSpace()->ContainsPointer(memory))
    {
        sj::MemorySystem::GetDebugMemSpace()->Free(memory);
    }
#endif
    else
    {
        sj::MemorySystem::GetUnmanagedMemSpace()->Free(memory);
    }
}

namespace sj {
    /** Used to track the active heap zone. */
    thread_local static_stack<IMemSpace*, 64> g_MemSpaceStack;

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

    void MemorySystem::Init()
    {
        // The first thing the program does once entering main should
        // be initializing the memory system, which happens on first access.
        Get();
    }

    MemorySystem* MemorySystem::Get()
    {
        static MemorySystem memSys;
        return &memSys;
    }

    void MemorySystem::PushMemSpace(IMemSpace* mem_space)
    {
        g_MemSpaceStack.push(mem_space);
    }

    void MemorySystem::PopMemSpace()
    {
        g_MemSpaceStack.pop();
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

        if(g_MemSpaceStack.empty())
        {
            return GetUnmanagedMemSpace();
        }
        else
        {
            return g_MemSpaceStack.top();
        }
    }
} // namespace sj
