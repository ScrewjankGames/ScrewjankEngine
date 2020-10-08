// STD Headers

// Library Headers

// Screwjank Headers
#include "core/MemorySystem.hpp"

#include "core/Assert.hpp"
#include "system/allocators/UnmanagedAllocator.hpp"
#include "system/allocators/FreeListAllocator.hpp"

namespace sj {
    MemorySystem::MemorySystem() : m_DefaultAllocator(nullptr)
    {
    }

    MemorySystem::~MemorySystem()
    {
        delete m_DefaultAllocator;
    }

    MemorySystem* MemorySystem::Get()
    {
        static MemorySystem memSys;
        return &memSys;
    }

    Allocator* MemorySystem::GetDefaultAllocator()
    {
        SJ_ASSERT(Get()->m_DefaultAllocator != nullptr,
                  "Memory system has not been initialized by engine yet.");
        return Get()->m_DefaultAllocator;
    }

    Allocator* MemorySystem::GetUnmanagedAllocator()
    {
        static UnmanagedAllocator s_Allocator;
        return &s_Allocator;
    }

    void MemorySystem::Initialize()
    {
        // Reserve 64MB of free space by default
        m_DefaultAllocator = new FreeListAllocator(67108864, GetUnmanagedAllocator());
    }
} // namespace sj
