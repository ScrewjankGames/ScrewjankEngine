module;

#include <ScrewjankStd/Assert.hpp>

#include <cstddef>
#include <cstdint>

export module sj.std.memory.allocators:StackAllocator;
import :Allocator;
import sj.std.memory.utils;


export namespace sj
{

    class StackAllocator final : public sj::memory_resource
    {
    public:
        /**
         * Constructor
         * @param buffer_size The size of this allocator's buffer in bytes
         * @param memory The memory managed by this allocator
         */
        StackAllocator(size_t buffer_size, void* memory)
        {
            init(buffer_size, memory);
        }

        /**
         * Destructor
         */
        ~StackAllocator() override = default;

        void init(size_t buffer_size, void* memory) override
        {
            m_BufferStart = memory;
            m_Offset = m_BufferStart;
            m_CurrentHeader = nullptr;
            m_Capacity = buffer_size;
        }

        /**
         * Frees most reset allocation
         */
        void pop_alloc()
        {
            void* currFrameAddr = reinterpret_cast<void*>(
                (reinterpret_cast<uintptr_t>(m_CurrentHeader) + sizeof(StackAllocatorHeader)));
            deallocate(currFrameAddr, 0, 0);
            return;
        }

        bool contains_ptr(void* memory) const override
        {
            return IsPointerInAddressSpace(
                memory,
                m_BufferStart,
                reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_BufferStart) + m_Capacity));
        }

    private:
        /** Data structure used to manage allocations the stack */
        struct StackAllocatorHeader
        {
            /** Pointer to the previous header block for popping */
            StackAllocatorHeader* PreviousHeader;

            /** Stores how many bytes were used to pad this header in the stack */
            size_t HeaderOffset;
        };

        /**
         * Allocates size bites from the heap
         * @param size The number of bytes to allocate
         * @param alignment The alignment requirement for this allocation
         */
        [[nodiscard]]
        void* do_allocate(const size_t size,
                          const size_t alignment = alignof(std::max_align_t)) override
        {
            SJ_ASSERT(m_BufferStart != nullptr, "Uninitialized allocator use.");

            // Calculate padding needed to align header and payload
            size_t alignment_requirement = std::max(alignment, alignof(StackAllocatorHeader));

            const uintptr_t currOffset = uintptr_t(m_Offset);
            // Get the padding required to align the header and payload in memory
            void* fist_possible_payload_address =
                reinterpret_cast<void*>(currOffset + sizeof(StackAllocatorHeader));

            size_t required_padding =
                GetAlignmentAdjustment(alignment_requirement, fist_possible_payload_address);

            size_t total_allocation_size = required_padding + sizeof(StackAllocatorHeader) + size;

            // Ensure the allocator has enough memory to satisfy the allocation
            auto free_space = m_Capacity - (currOffset - uintptr_t(m_BufferStart));
            if(free_space < total_allocation_size)
            {
                SJ_ENGINE_LOG_ERROR(
                    "Stack allocator has insufficient memory to perform requested allocation.");
                return nullptr;
            }

            // Allocate the header
            void* header_memory = reinterpret_cast<void*>(currOffset + required_padding);

            SJ_ASSERT(IsMemoryAligned(header_memory, alignof(StackAllocatorHeader)),
                      "Attempting to place allocation header at unaligned address!");

            auto old_header = m_CurrentHeader;

            m_CurrentHeader =
                new(header_memory) StackAllocatorHeader {.PreviousHeader = old_header,
                                                         .HeaderOffset = required_padding};

            // Update m_offset
            m_Offset = reinterpret_cast<void*>(currOffset + total_allocation_size);

            // Return pointer to the start of the allocation
            return reinterpret_cast<void*>(uintptr_t(header_memory) + sizeof(StackAllocatorHeader));
        }

        /**
         * Marks memory as free
         * @param memory Pointer to the memory to free
         * @note memory must point to the head of the most recent allocation, or nullptr to pop off
         * the stack
         */
        void do_deallocate(void* memory,
                           [[maybe_unused]] size_t bytes,
                           [[maybe_unused]] size_t alignment) override
        {
            SJ_ASSERT(m_CurrentHeader != nullptr, "You cannot free from an empty stack allocator");

            const uintptr_t headerLoc = reinterpret_cast<uintptr_t>(m_CurrentHeader);

            // The memory argument is ignored unless it is a specific value
            if(memory != nullptr)
            {
                SJ_ASSERT(reinterpret_cast<uintptr_t>(memory) ==
                              headerLoc + sizeof(StackAllocatorHeader),
                          "Stack allocator cannot free memory that is not on top of the stack.");
            }

            // Get beginning of "top" allocation frame
            auto current_frame_start = headerLoc - m_CurrentHeader->HeaderOffset;

            // Roll free memory pointer back to start of current frame
            m_Offset = reinterpret_cast<void*>(current_frame_start);

            // Roll header back to previous header
            m_CurrentHeader = m_CurrentHeader->PreviousHeader;

            return;
        }

        /** The size in bytes of the allocator's buffer */
        size_t m_Capacity = 0;

        /** Buffer the allocator manages */
        void* m_BufferStart = nullptr;

        /** Pointer to the latest allocation's header block */
        StackAllocatorHeader* m_CurrentHeader = nullptr;

        /** Pointer to the first free byte in the stack */
        void* m_Offset = nullptr;
    };

} // namespace sj
