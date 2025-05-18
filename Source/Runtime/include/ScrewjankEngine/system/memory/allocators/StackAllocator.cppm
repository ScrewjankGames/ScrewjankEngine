module;

#include <ScrewjankShared/utils/Assert.hpp>
#include <ScrewjankShared/utils/MemUtils.hpp>

#include <cstddef>
#include <cstdint>

export module sj.engine.system.memory.allocators:StackAllocator;
import :Allocator;

export namespace sj
{

    class StackAllocator final : public Allocator
    {
    public:
        /**
         * Constructor
         * @param buffer_size The size of this allocator's buffer in bytes
         * @param memory The memory managed by this allocator
         */
        StackAllocator(size_t buffer_size, void* memory)
        {
            Init(buffer_size, memory);
        }

        /**
         * Destructor
         */
        ~StackAllocator() override = default;

        void Init(size_t buffer_size, void* memory)
        {
            m_BufferStart = memory;
            m_Offset = m_BufferStart;
            m_CurrentHeader = nullptr;
            m_Capacity = buffer_size;
        }

        /**
         * Allocates size bites from the heap
         * @param size The number of bytes to allocate
         * @param alignment The alignment requirement for this allocation
         */
        [[nodiscard]]
        void* allocate(const size_t size,
                       const size_t alignment = alignof(std::max_align_t)) override
        {
            SJ_ASSERT(m_BufferStart != nullptr, "Uninitialized allocator use.");

            // Calculate padding needed to align header and payload
            size_t alignment_requirement = std::max(alignment, alignof(StackAllocatorHeader));

            // Get the padding required to align the header and payload in memory
            void* fist_possible_payload_address =
                (void*)((uintptr_t)m_Offset + sizeof(StackAllocatorHeader));

            size_t required_padding =
                GetAlignmentAdjustment(alignment_requirement, fist_possible_payload_address);

            size_t total_allocation_size = required_padding + sizeof(StackAllocatorHeader) + size;

            // Ensure the allocator has enough memory to satisfy the allocation
            auto free_space = m_Capacity - (uintptr_t(m_Offset) - uintptr_t(m_BufferStart));
            if(free_space < total_allocation_size)
            {
                SJ_ENGINE_LOG_ERROR(
                    "Stack allocator has insufficient memory to perform requested allocation.");
                return nullptr;
            }

            // Allocate the header
            void* header_memory = (void*)((uintptr_t)m_Offset + required_padding);

            SJ_ASSERT(IsMemoryAligned(header_memory, alignof(StackAllocatorHeader)),
                      "Attempting to place allocation header at unaligned address!");

            auto old_header = m_CurrentHeader;

            m_CurrentHeader =
                new((void*)header_memory) StackAllocatorHeader {old_header, required_padding};

            // Update m_offset
            m_Offset = (void*)(uintptr_t(m_Offset) + total_allocation_size);

            // Return pointer to the start of the allocation
            return (void*)((uintptr_t)header_memory + sizeof(StackAllocatorHeader));
        }

        /**
         * Marks memory as free
         * @param memory Pointer to the memory to free
         * @note memory must point to the head of the most recent allocation, or nullptr to pop off
         * the stack
         */
        void deallocate(void* memory = nullptr) override
        {
            SJ_ASSERT(m_CurrentHeader != nullptr, "You cannot free from an empty stack allocator");

            // The memory argument is ignored unless it is a specific value
            if(memory != nullptr)
            {
                SJ_ASSERT((uintptr_t)memory ==
                              (uintptr_t)m_CurrentHeader + sizeof(StackAllocatorHeader),
                          "Stack allocator cannot free memory that is not on top of the stack.");
            }

            // Get beginning of "top" allocation frame
            auto current_frame_start = (uintptr_t)m_CurrentHeader - m_CurrentHeader->HeaderOffset;

            // Roll free memory pointer back to start of current frame
            m_Offset = (void*)current_frame_start;

            // Roll header back to previous header
            m_CurrentHeader = m_CurrentHeader->PreviousHeader;

            return;
        }

        /**
         * See Allocate
         */
        [[nodiscard]] void* PushAlloc(size_t size, size_t alignment = alignof(std::max_align_t));

        /**
         * Frees most reset allocation
         */
        void PopAlloc()
        {
            deallocate(nullptr);
            return;
        }

        uintptr_t Begin() const override
        {
            return reinterpret_cast<uintptr_t>(m_BufferStart);
        }

        uintptr_t End() const override
        {
            return reinterpret_cast<uintptr_t>(m_BufferStart) + m_Capacity;
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

        /** The size in bytes of the allocator's buffer */
        size_t m_Capacity = 0;

        /** Buffer the allocator manages */
        void* m_BufferStart = nullptr;

        /** Pointer to the latest allocation's header block */
        StackAllocatorHeader* m_CurrentHeader = nullptr;

        /** Pointer to the first free byte in the stack */
        void* m_Offset = nullptr;

        /** Structure used to track and report the state of this allocator */
        AllocatorStatus m_AllocatorStats;
    };

} // namespace sj
