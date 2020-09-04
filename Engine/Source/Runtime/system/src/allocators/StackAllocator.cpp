// STD Headers
#include <cassert>

// Library Headers

// Engine Headers
#include "system/allocators/StackAllocator.hpp"
#include "system/Memory.hpp"

namespace Screwjank {

    StackAllocator::StackAllocator(size_t buffer_size)
        : m_CurrFrameHeader(nullptr), m_Capacity(buffer_size), m_Used(0)
    {
        m_Buffer = (std::byte*)GlobalAllocator()->Allocate(buffer_size);

        // Place a header at the start of the buffer
        m_CurrFrameHeader = new (m_Buffer) StackAllocatorHeader {nullptr};
    }

    StackAllocator::~StackAllocator()
    {
        GlobalAllocator()->Free(m_Buffer);
    }

    void* Screwjank::StackAllocator::Allocate(const size_t size, const size_t alignment)
    {

        return nullptr;
    }

    void StackAllocator::Free(void* memory)
    {
        assert(memory == nullptr);
    }

    void StackAllocator::Pop()
    {
    }

    void StackAllocator::Reset()
    {
        m_CurrFrameHeader = new (m_Buffer) StackAllocatorHeader {nullptr};
    }

} // namespace Screwjank
