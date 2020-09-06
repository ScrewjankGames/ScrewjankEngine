// STD Headers
#include <memory>
#include <cassert>

// Library Headers
#include "core/Assert.hpp"
#include "core/Log.hpp"
#include "system/Memory.hpp"

// Screwjank Headers

using namespace Screwjank;

void* operator new(size_t num_bytes) noexcept(false)
{
    return GlobalAllocator()->Allocate(num_bytes);
}

void operator delete(void* memory) throw()
{
    GlobalAllocator()->Free(memory);
}

namespace Screwjank {

    Allocator* GlobalAllocator()
    {
        static Screwjank::BasicAllocator s_Allocator;
        return &s_Allocator;
    }

    void* AlignMemory(size_t align_of, size_t size, void* buffer_start, size_t buffer_size)
    {
        if (align_of == 0) {
            return buffer_start;
        }

        // try to carve out _Size bytes on boundary _Bound
        size_t offset = (size_t)((uintptr_t)(buffer_start) & (align_of - 1));

        if (offset != 0) {
            offset = align_of - offset; // number of bytes to skip
        }

        assert(!(buffer_size < offset) || !(buffer_size - offset < size));

        // enough room, update
        buffer_start = (void*)((uintptr_t)buffer_start + offset);
        return buffer_start;
    }

    char MemorySystem::s_GlobalAllocatorBuffer[sizeof(BasicAllocator)];
    Allocator* MemorySystem::s_GlobalAllocator;

    void MemorySystem::Initialize()
    {
        s_GlobalAllocator = new (s_GlobalAllocatorBuffer) BasicAllocator();
    }

    void MemorySystem::LogStatus()
    {
        SJ_LOG_INFO("TODO Memory system status:")
    }

} // namespace Screwjank
