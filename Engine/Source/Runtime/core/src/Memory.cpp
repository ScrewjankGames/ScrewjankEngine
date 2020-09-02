// STD Headers
#include <memory>
#include <cassert>

// Library Headers
#include "core/Log.hpp"

// Screwjank Headers

void* operator new(size_t num_bytes) noexcept(false)
{
    SJ_ENGINE_LOG_INFO("Heap allocating %zd bytes", num_bytes);

    void* memory = malloc(num_bytes);
    return memory;
}

void operator delete(void* memory) throw()
{
    SJ_ENGINE_LOG_INFO("Freeing memory at address 0x%p", memory);

    free(memory);
}
