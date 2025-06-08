module;
#include <cstddef>
#include <new>
#include <memory_resource>

export module sj.engine.system.memory.NewDelete;
import sj.engine.system.memory.MemorySystem;
import sj.std.memory.resources;

extern "C++" {
[[nodiscard]] void* do_allocate(size_t count, size_t alignment = alignof(std::max_align_t))
{
    if(count == 0)
        count++;

    std::pmr::memory_resource* resource = sj::MemorySystem::GetCurrentMemoryResource();
    return resource->allocate(count, alignment);
}

[[nodiscard]] void* operator new(std::size_t count) noexcept(false)
{
    return do_allocate(count);
}

[[nodiscard]] void* operator new[](std::size_t count) noexcept(false)
{
    return do_allocate(count);
}

[[nodiscard]] void* operator new(std::size_t count, std::align_val_t al) noexcept(false)
{
    return do_allocate(count, static_cast<std::size_t>(al));
}

[[nodiscard]] void* operator new[](std::size_t count, std::align_val_t al) noexcept(false)
{
    return do_allocate(count, static_cast<std::size_t>(al));
}

[[nodiscard]] void* operator new(std::size_t count, const std::nothrow_t& _) noexcept
{
    return do_allocate(count);
}

[[nodiscard]] void* operator new[](std::size_t count, const std::nothrow_t& _) noexcept
{
    return do_allocate(count);
}

[[nodiscard]] void*
operator new(std::size_t count, std::align_val_t al, const std::nothrow_t& _) noexcept
{
    return do_allocate(count, static_cast<std::size_t>(al));
}

[[nodiscard]] void*
operator new[](std::size_t count, std::align_val_t al, const std::nothrow_t& _) noexcept
{
    return do_allocate(count, static_cast<std::size_t>(al));
}

void do_deallocate(void* ptr,
                   std::size_t sz,
                   [[maybe_unused]] std::size_t align = alignof(std::max_align_t)) noexcept
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

void operator delete[](void* ptr) noexcept
{
    do_deallocate(ptr, 0);
}

void operator delete(void* ptr, std::size_t sz) noexcept
{
    do_deallocate(ptr, sz);
}

void operator delete[](void* ptr, std::size_t sz) noexcept
{
    do_deallocate(ptr, sz);
}

void operator delete(void* ptr, std::size_t sz, std::align_val_t _) noexcept
{
    do_deallocate(ptr, sz);
}

void operator delete[](void* ptr, std::size_t sz, std::align_val_t _) noexcept
{
    do_deallocate(ptr, sz);
}

}