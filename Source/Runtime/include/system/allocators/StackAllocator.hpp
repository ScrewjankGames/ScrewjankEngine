#pragma once
// STD Headers
#include <cstddef>

// Library Headers

// Engine Headers
#include <system/Allocator.hpp>
#include <system/Memory.hpp>

namespace sj {

    class StackAllocator : public Allocator
    {
      public:
        /**
         * Constructor
         * @param buffer_size The size of this allocator's buffer in bytes
         * @param memory The memory managed by this allocator
         */
        StackAllocator(size_t buffer_size, void* memory);
        
        /**
         * Destructor
         */
        ~StackAllocator();

        /**
         * Allocates size bites from the heap
         * @param size The number of bytes to allocate
         * @param alignment The alignment requirement for this allocation
         */
        [[nodiscard]] void* Allocate(const size_t size,
                                     const size_t alignment = alignof(std::max_align_t)) override;

        /**
         * Marks memory as free
         * @param memory Pointer to the memory to free
         * @note memory must point to the head of the most recent allocation, or nullptr to pop off
         * the stack
         */
        void Free(void* memory = nullptr) override;

        /**
         * See Allocate
         */
        [[nodiscard]] void* PushAlloc(size_t size, size_t alignment = alignof(std::max_align_t));

        /**
         * Calls Free() with the address of the last allocation
         */
        void PopAlloc();

        /**
         * Calls allocate with the correct size and alignment for T
         */
        template <typename T>
        [[nodiscard]] void* PushType();

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
        size_t m_Capacity;

        /** Buffer the allocator manages */
        void* m_BufferStart;

        /** Pointer to the latest allocation's header block */
        StackAllocatorHeader* m_CurrentHeader;

        /** Pointer to the first free byte in the stack */
        void* m_Offset;

        /** Structure used to track and report the state of this allocator */
        AllocatorStatus m_AllocatorStats;
    };

    template <typename T>
    inline void* StackAllocator::PushType()
    {
        return Allocate(sizeof(T), alignof(T));
    }
} // namespace sj
