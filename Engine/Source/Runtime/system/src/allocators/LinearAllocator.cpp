// STD Headers
#include <cassert>

// Library Headers

// Engine Headers
#include "system/allocators/LinearAllocator.hpp"
#include "system/Memory.hpp"

namespace Screwjank {
    LinearAllocator::LinearAllocator(size_t buffer_size) : m_Capacity(buffer_size)
    {
        m_Buffer = GlobalAllocator()->Allocate(buffer_size);
        m_CurrFrameStart = m_Buffer;
    }

    LinearAllocator::~LinearAllocator()
    {
        GlobalAllocator()->Free(m_Buffer);
    }

    void* LinearAllocator::Allocate(const size_t size, const size_t alignment)
    {
        size_t space = uintptr_t(m_Buffer) + m_Capacity - uintptr_t(m_CurrFrameStart);
        auto allocated_memory = AlignMemory(alignment, size, m_CurrFrameStart, space);

        m_CurrFrameStart = (void*)((uintptr_t)allocated_memory + size + 1);

        return allocated_memory;
    }

    void LinearAllocator::Free(void* memory)
    {
        assert(false, "Linear allocators do not allow memory to be freed.");
    }

    void LinearAllocator::Reset()
    {
        m_CurrFrameStart = m_Buffer;
    }
} // namespace Screwjank
