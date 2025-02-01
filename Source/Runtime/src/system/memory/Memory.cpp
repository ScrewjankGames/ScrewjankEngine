// STD Headers
#include <cassert>

// Screwjank Headers
#include <ScrewjankEngine/containers/StaticStack.hpp>
#include <ScrewjankShared/utils/Assert.hpp>
#include <ScrewjankEngine/utils/Log.hpp>
#include <ScrewjankEngine/system/memory/Memory.hpp>
#include <ScrewjankEngine/system/memory/allocators/FreeListAllocator.hpp>

// Root heap sizes
constexpr uint64_t kRootHeapSize = sj::k1_MiB * 1;
constexpr uint64_t kDebugHeapSize = sj::k1_MiB * 64;

[[nodiscard]] void* operator new(size_t num_bytes) noexcept(false)
{
    return sj::MemorySystem::GetCurrentMemSpace()->Allocate(num_bytes);
}

void operator delete(void* memory) noexcept
{
    if (sj::MemorySystem::GetCurrentMemSpace()->ContainsPointer(memory))
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
#ifndef GOLD_VERSION
          , m_DebugMemSpace(nullptr, kDebugHeapSize, "Debug Heap")
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
            return GetRootMemSpace();
        }
        else
        {
            return g_MemSpaceStack.Top();
        }
    }



    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// Memory Management Utility Functions
    ///////////////////////////////////////////////////////////////////////////////////////////////



} // namespace sj
