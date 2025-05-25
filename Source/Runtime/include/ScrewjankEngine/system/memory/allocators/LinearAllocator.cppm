module;
#include <ScrewjankShared/utils/Log.hpp>
#include <ScrewjankShared/utils/Assert.hpp>

export module sj.engine.system.memory.allocators:LinearAllocator;
import :Allocator;
import sj.engine.system.memory.utils;

export namespace sj
{
    class LinearAllocator final : public sj::memory_resource
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
            init(buffer_size, memory);
        }

        void init(size_t buffer_size, void* memory) override
        {
            m_BufferStart = memory;
            m_BufferEnd =
                reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_BufferStart) + buffer_size);

            m_CurrFrameStart = memory;
        }

        /**
         * Destructor
         */
        ~LinearAllocator() final = default;

        /**
         * Marks all allocations for this allocator as invalid and frees the buffer for more
         * allocations
         */
        void reset()
        {
            m_CurrFrameStart = m_BufferStart;
        }

        /**
         * @return whether the allocator is in a valid state
         */
        [[nodiscard]] bool is_initialized() const
        {
            return m_BufferStart != nullptr;
        }

        bool contains_ptr(void* memory) const override
        {
            return IsPointerInAddressSpace(memory, m_BufferStart, m_BufferEnd);
        }

    private:
        [[nodiscard]] void* do_allocate(const size_t size,
                                        const size_t alignment = alignof(std::max_align_t)) override
        {
            size_t free_space = uintptr_t(m_BufferEnd) - uintptr_t(m_CurrFrameStart);

            SJ_ASSERT(is_initialized(), "Trying to allocate with uninitialized allocator!");

            // Ensure there is enough space to satisfy allocation
            if(free_space < size + GetAlignmentOffset(alignment, m_CurrFrameStart))
            {
                SJ_ENGINE_LOG_FATAL(
                    "Allocator has insufficient memory to perform requested allocation");
                return nullptr;
            }

            auto num_bytes_allocated = size + GetAlignmentOffset(alignment, m_CurrFrameStart);
            auto allocated_memory = AlignMemory(alignment, size, m_CurrFrameStart, free_space);

            SJ_ASSERT(num_bytes_allocated <= free_space, "Linear Allocator is out of memory!");

            // Bump allocation pointer to the first free byte after the current allocation
            m_CurrFrameStart =
                reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(allocated_memory) + size);

            return allocated_memory;
        }

        /**
         * Attempts to free a memory address
         * @note Linear Allocators don't support the free operation, expected to just call reset
         * eventually
         */
        void do_deallocate([[maybe_unused]] void* memory,
                           [[maybe_unused]] size_t bytes,
                           [[maybe_unused]] size_t alignment) override
        {
            ;
        }

        /** Pointer to the start of the allocator's managed memory */
        void* m_BufferStart = nullptr;

        /** Pointer to the end of the allocator's managed memory */
        void* m_BufferEnd = nullptr;

        /** Pointer to the first free byte in the linear allocator */
        void* m_CurrFrameStart = nullptr;
    };

} // namespace sj
