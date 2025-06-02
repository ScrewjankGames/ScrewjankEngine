module;

#include <memory_resource>

export module sj.engine.system.memory.NewDelete;
import sj.engine.system.memory.MemorySystem;
import sj.std.memory.allocators;

extern "C++" {
[[nodiscard]] void* operator new(size_t num_bytes) noexcept(false)
{
    if(num_bytes == 0)
        num_bytes++;

    std::pmr::memory_resource* resource = sj::MemorySystem::GetCurrentMemoryResource();
    return resource->allocate(num_bytes);
}

void do_deallocate(void* ptr, std::size_t sz) noexcept
{

    sj::memory_resource* owning_resource = sj::MemorySystem::FindOwningResource(ptr);
    if(owning_resource)
        owning_resource->deallocate(ptr, sz);
    else
        sj::MemorySystem::GetUnmanagedMemoryResource()->deallocate(ptr, sz);
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