// STD Headers
#include <memory>
#include <cassert>

// Library Headers
#include "core/Log.hpp"

// Screwjank Headers

void* operator new(size_t num_bytes) noexcept(false)
{
    void* memory = malloc(num_bytes);
    return memory;
}

void operator delete(void* memory) throw()
{
    free(memory);
}
