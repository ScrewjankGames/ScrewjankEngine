// STD Headers

// Library Headers

// Screwjank Headers
#include "core/MemorySystem.hpp"

#include "core/Assert.hpp"
#include "system/allocators/UnmanagedAllocator.hpp"

namespace Screwjank {
    MemorySystem::MemorySystem() : m_DefaultAllocator(nullptr)
    {
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

    Allocator* MemorySystem::GetDefaultUnmanagedAllocator()
    {
        static Screwjank::UnmanagedAllocator s_Allocator;
        return &s_Allocator;
    }

    void MemorySystem::Initialize()
    {
        m_DefaultAllocator = new UnmanagedAllocator();
    }
} // namespace Screwjank
