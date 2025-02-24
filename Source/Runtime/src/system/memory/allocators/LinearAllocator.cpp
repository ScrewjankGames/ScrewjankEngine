// Parent Include
#include <ScrewjankEngine/system/memory/allocators/LinearAllocator.hpp>

// STD Headers

// Library Headers

// Screwjank Headers
#include <ScrewjankShared/utils/Assert.hpp>
#include <ScrewjankShared/utils/Log.hpp>
#include <ScrewjankEngine/system/memory/Memory.hpp>
#include <ScrewjankShared/utils/MemUtils.hpp>

namespace sj {

    LinearAllocator::LinearAllocator(size_t buffer_size, void* memory)
    {
        Init(buffer_size, memory);
    }

    LinearAllocator::~LinearAllocator()
    {
        // While other allocators should assert, linear allocators are perfectly capable of freeing
        // all of their allocations on destruction
        if (!(m_AllocatorStats.ActiveAllocationCount == 0 &&
              m_AllocatorStats.FreeSpace() == m_AllocatorStats.Capacity)) {
            SJ_ENGINE_LOG_WARN("Linear allocator was not properly reset before destruction.");
            Reset();
        }
    }

    void LinearAllocator::Init(size_t buffer_size, void* memory)
    {
        m_BufferStart = memory;
        m_BufferEnd =
            reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_BufferStart) + buffer_size);

        m_CurrFrameStart = memory;
        m_AllocatorStats.Capacity = buffer_size;
    }

    void* LinearAllocator::Allocate(const size_t size, const size_t alignment)
    {
        size_t free_space = m_AllocatorStats.FreeSpace();

        SJ_ASSERT(IsInitialized(), "Trying to allocate with uninitialized allocator!");

        // Ensure there is enough space to satisfy allocation
        if (free_space < size + GetAlignmentOffset(alignment, m_CurrFrameStart))
        {
            SJ_ENGINE_LOG_FATAL("Allocator has insufficient memory to perform requested allocation");
            return nullptr;
        }

        auto num_bytes_allocated = size + GetAlignmentOffset(alignment, m_CurrFrameStart);
        auto allocated_memory = AlignMemory(alignment, size, m_CurrFrameStart, free_space);

        SJ_ASSERT(num_bytes_allocated <= m_AllocatorStats.FreeSpace(), "Linear Allocator is out of memory!");

        // Bump allocation pointer to the first free byte after the current allocation
        m_CurrFrameStart = (void*)((uintptr_t)allocated_memory + size);

        // Track allocation data
        m_AllocatorStats.ActiveAllocationCount++;
        m_AllocatorStats.TotalAllocationCount++;
        m_AllocatorStats.TotalBytesAllocated += num_bytes_allocated;
        m_AllocatorStats.ActiveBytesAllocated += num_bytes_allocated;


        return allocated_memory;
    }

    void* LinearAllocator::Reallocate(void* originalPtr, const size_t size, const size_t alignment)
    {
        SJ_ASSERT(false, "Invalid Operation");
        return nullptr;
    }

    void LinearAllocator::Free(void* memory)
    {
        SJ_ASSERT(false, "Linear allocators do not allow memory to be freed.");
    }

    void LinearAllocator::Reset()
    {
        SJ_ASSERT(IsInitialized(), "Trying to reset uninitialized allocator!");

        // Track allocation data
        m_AllocatorStats.ActiveAllocationCount = 0;
        m_AllocatorStats.ActiveBytesAllocated = 0;
        m_CurrFrameStart = m_BufferStart;
    }

    bool LinearAllocator::IsInitialized() const
    {
        return m_BufferStart != nullptr;
    }

    uintptr_t LinearAllocator::Begin() const
    {
        return reinterpret_cast<uintptr_t>(m_BufferStart);
    }

    uintptr_t LinearAllocator::End() const 
    {
        return reinterpret_cast<uintptr_t>(m_BufferEnd);
    }

} // namespace sj
