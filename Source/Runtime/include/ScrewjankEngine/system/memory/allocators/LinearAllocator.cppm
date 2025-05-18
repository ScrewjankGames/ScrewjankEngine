module;
#include <ScrewjankShared/utils/Log.hpp>
#include <ScrewjankShared/utils/Assert.hpp>
#include <ScrewjankShared/utils/MemUtils.hpp>

export module sj.engine.system.memory.allocators:LinearAllocator;
import :Allocator;

export namespace sj
{

    class LinearAllocator final : public Allocator
    {
    public:
        /**
         * Default constructor
         */
        LinearAllocator() = default;

        /**
         * Initializing Constructor
         */
        explicit LinearAllocator(size_t buffer_size, void* memory)
        {
            Init(buffer_size, memory);
        }

        /**
         * Destructor
         */
        ~LinearAllocator()
        {
            // While other allocators should assert, linear allocators are perfectly capable of
            // freeing all of their allocations on destruction
            if(!(m_AllocatorStats.ActiveAllocationCount == 0 &&
                 m_AllocatorStats.FreeSpace() == m_AllocatorStats.Capacity))
            {
                SJ_ENGINE_LOG_WARN("Linear allocator was not properly reset before destruction.");
                Reset();
            }
        }

        /**
         * Allocates size bites from the heap
         * @param size The number of bytes to allocate
         * @param alignment The alignment requirement for this allocation
         */
        [[nodiscard]] void* allocate(const size_t size,
                                     const size_t alignment = alignof(std::max_align_t)) override
        {
            size_t free_space = m_AllocatorStats.FreeSpace();

            SJ_ASSERT(IsInitialized(), "Trying to allocate with uninitialized allocator!");

            // Ensure there is enough space to satisfy allocation
            if(free_space < size + GetAlignmentOffset(alignment, m_CurrFrameStart))
            {
                SJ_ENGINE_LOG_FATAL(
                    "Allocator has insufficient memory to perform requested allocation");
                return nullptr;
            }

            auto num_bytes_allocated = size + GetAlignmentOffset(alignment, m_CurrFrameStart);
            auto allocated_memory = AlignMemory(alignment, size, m_CurrFrameStart, free_space);

            SJ_ASSERT(num_bytes_allocated <= m_AllocatorStats.FreeSpace(),
                      "Linear Allocator is out of memory!");

            // Bump allocation pointer to the first free byte after the current allocation
            m_CurrFrameStart = (void*)((uintptr_t)allocated_memory + size);

            // Track allocation data
            m_AllocatorStats.ActiveAllocationCount++;
            m_AllocatorStats.TotalAllocationCount++;
            m_AllocatorStats.TotalBytesAllocated += num_bytes_allocated;
            m_AllocatorStats.ActiveBytesAllocated += num_bytes_allocated;

            return allocated_memory;
        }

        /*
         * @param buffer_size The size (in bytes) of the memory buffer being managed
         * @param memory The memory this allocator should manage
         */
        void Init(size_t buffer_size, void* memory)
        {
            m_BufferStart = memory;
            m_BufferEnd =
                reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_BufferStart) + buffer_size);

            m_CurrFrameStart = memory;
            m_AllocatorStats.Capacity = buffer_size;
        }

        /**
         * Attempts to free a memory address
         * @param memory Pointer to the memory to free
         * @note Linear Allocators don't support the free operation, and will assert failure if
         * called
         */
        void deallocate(void* memory = nullptr) override
        {
            ;
        }

        /**
         * Marks all allocations for this allocator as invalid and frees the buffer for more
         * allocations
         */
        void Reset()
        {
            SJ_ASSERT(IsInitialized(), "Trying to reset uninitialized allocator!");

            // Track allocation data
            m_AllocatorStats.ActiveAllocationCount = 0;
            m_AllocatorStats.ActiveBytesAllocated = 0;
            m_CurrFrameStart = m_BufferStart;
        }

        /**
         * @return whether the allocator is in a valid state
         */
        bool IsInitialized() const
        {
            return m_BufferStart != nullptr;
        }

        uintptr_t Begin() const override
        {
            return reinterpret_cast<uintptr_t>(m_BufferStart);
        }
        uintptr_t End() const override
        {
            return reinterpret_cast<uintptr_t>(m_BufferEnd);
        }

    private:
        /** Pointer to the start of the allocator's managed memory */
        void* m_BufferStart;

        /** Pointer to the end of the allocator's managed memory */
        void* m_BufferEnd;

        /** Pointer to the first free byte in the linear allocator */
        void* m_CurrFrameStart;

        /** Structure used to track and report the state of this allocator */
        AllocatorStatus m_AllocatorStats;
    };

} // namespace sj
