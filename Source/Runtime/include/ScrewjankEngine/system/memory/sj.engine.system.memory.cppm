module;

#include <memory>
#include <cstdint>
#include <cstddef>
#include <new>

#include <ScrewjankShared/utils/Assert.hpp>

export module sj.engine.system.memory;
export import :Literals;
export import :MemSpace;
export import :MemorySystem;

export import sj.engine.system.memory.allocators;

module :private;
extern "C++" {
[[nodiscard]] void* operator new(size_t num_bytes) noexcept(false)
{
    if(num_bytes == 0)
        num_bytes++;

    return sj::MemorySystem::GetCurrentMemSpace()->allocate(num_bytes);
}

void do_deallocate(void* ptr, std::size_t sz) noexcept
{
    sj::IMemSpace* currMemSpace = sj::MemorySystem::GetCurrentMemSpace();
    sj::IMemSpace* unmanagedMemSpace = sj::MemorySystem::GetUnmanagedMemSpace();

    if(currMemSpace && currMemSpace != unmanagedMemSpace && currMemSpace->contains_ptr(ptr))
    {
        currMemSpace->deallocate(ptr, sz);
    }
    else if(sj::MemorySystem::GetRootMemSpace()->contains_ptr(ptr))
    {
        // Search for correct MemSpace for pointer supplied
        sj::IMemSpace* mem_space = sj::find_mem_space(ptr);
        SJ_ASSERT(mem_space != nullptr, "Failed to find managed memory region");
        mem_space->deallocate(ptr, sz);
    }
#ifndef GOLD_VERSION
    else if(sj::MemorySystem::GetDebugMemSpace()->contains_ptr(ptr))
    {
        sj::MemorySystem::GetDebugMemSpace()->deallocate(ptr, sz);
    }
#endif
    else
    {
        sj::MemorySystem::GetUnmanagedMemSpace()->deallocate(ptr, sz);
    }
}

void operator delete(void* ptr) noexcept
{
    do_deallocate(ptr, 0);
}

void operator delete(void* ptr, std::size_t sz) noexcept
{
    do_deallocate(ptr, sz);
}
}