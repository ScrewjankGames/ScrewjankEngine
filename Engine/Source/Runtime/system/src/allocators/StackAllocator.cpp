// STD Headers

// Library Headers

// Engine Headers
#include "system/allocators/StackAllocator.hpp"
#include "system/Memory.hpp"

namespace Screwjank {

    StackAllocator::StackAllocator(size_t buffer_size,
                                   Allocator* backing_allocator,
                                   const char* debug_name)
        : Allocator(backing_allocator, debug_name), m_BackingAllocator(backing_allocator),
          m_CurrFrameHeader(nullptr), m_Capacity(buffer_size)
    {
        m_BackingAllocator->Allocate(buffer_size);
    }

    StackAllocator::~StackAllocator()
    {
        m_BackingAllocator->Free(m_Buffer);
    }

    void* Screwjank::StackAllocator::Allocate(const size_t size, const size_t alignment)
    {

        return nullptr;
    }

    void StackAllocator::Free(void* memory)
    {
    }

    void StackAllocator::Pop()
    {
    }

    void StackAllocator::Reset()
    {
    }

} // namespace Screwjank
