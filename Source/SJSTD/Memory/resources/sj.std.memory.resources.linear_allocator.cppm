module;
#include <ScrewjankStd/Log.hpp>
#include <ScrewjankStd/Assert.hpp>

#include <cstddef>
#include <cstdint>

export module sj.std.memory.resources.linear_allocator;
import sj.std.memory.resources.memory_resource;
import sj.std.memory.utils;

export namespace sj
{
    class linear_allocator final : public sj::memory_resource
    {
    public:
        linear_allocator() = default;

        explicit linear_allocator(size_t buffer_size, std::byte* memory)
        {
            init(buffer_size, memory);
        }

        void init(size_t buffer_size, std::byte* memory) override
        {
            m_BufferStart = memory;
            m_BufferEnd =
                reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_BufferStart) + buffer_size);

            m_CurrFrameStart = memory;
        }

        ~linear_allocator() final = default;

        auto get_current_offset() -> size_t
        {
            return uintptr_t(m_CurrFrameStart) - uintptr_t(m_BufferStart);
        }

        void reset()
        {
            m_CurrFrameStart = m_BufferStart;
        }

        void reset(size_t to_offset)
        {
            m_CurrFrameStart = reinterpret_cast<void*>(uintptr_t(m_BufferStart) + to_offset);
        }

        [[nodiscard]] bool is_initialized() const
        {
            return m_BufferStart != nullptr;
        }

        bool contains_ptr(void* memory) const override
        {
            return IsPointerInAddressSpace(memory, m_BufferStart, m_BufferEnd);
        }

        void* data() 
        {
            return m_BufferStart;
        }

        size_t buffer_size() const
        {
            return uintptr_t(m_BufferEnd) - uintptr_t(m_BufferStart);
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

            auto allocated_memory = AlignMemory(alignment, size, m_CurrFrameStart, free_space);

            SJ_ASSERT(uintptr_t(allocated_memory) + size <= uintptr_t(m_BufferEnd), "Linear Allocator is out of memory!");

            // Bump allocation pointer to the first free byte after the current allocation
            m_CurrFrameStart =
                reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(allocated_memory) + size);

            return allocated_memory;
        }

        /**
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
