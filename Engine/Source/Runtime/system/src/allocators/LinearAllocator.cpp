// STD Headers

// Library Headers

// Engine Headers
#include "core/Assert.hpp"
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
        SJ_ASSERT(space != 0, "Allocator is out of memory");
        SJ_ASSERT(space >= size, "Allocator has insufficient memory for allocation");

        auto allocated_memory = AlignMemory(alignment, size, m_CurrFrameStart, space);

        if ((uintptr_t)allocated_memory + size > (uintptr_t)m_Buffer + m_Capacity) {
            SJ_ASSERT(false, "Allocator has insufficient memory to align allocation!");
            return nullptr;
        }

        m_CurrFrameStart = (void*)((uintptr_t)allocated_memory + size);
        return allocated_memory;
    }

    void LinearAllocator::Free(void* memory)
    {
        SJ_ASSERT(false, "Linear allocators do not allow memory to be freed.");
    }

    void LinearAllocator::Reset()
    {
        m_CurrFrameStart = m_Buffer;
    }
} // namespace Screwjank
