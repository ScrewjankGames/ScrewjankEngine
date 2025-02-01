#pragma once
// STD Headers

// Library Headers

// Engine Headers
#include <ScrewjankEngine/system/memory/Allocator.hpp>

namespace sj {

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
        explicit LinearAllocator(size_t buffer_size, void* memory);

        /**
         * Destructor
         */
        ~LinearAllocator();

        /**
         * Allocates size bites from the heap
         * @param size The number of bytes to allocate
         * @param alignment The alignment requirement for this allocation
         */
        [[nodiscard]] void* Allocate(const size_t size,
                                     const size_t alignment = alignof(std::max_align_t)) override;

        /*
         * @param buffer_size The size (in bytes) of the memory buffer being managed
         * @param memory The memory this allocator should manage
         */
        void Init(size_t buffer_size, void* memory) override;

        /**
         * Attempts to free a memory address
         * @param memory Pointer to the memory to free
         * @note Linear Allocators don't support the free operation, and will assert failure if
         * called
         */
        void Free(void* memory = nullptr) override;

        /**
         * Marks all allocations for this allocator as invalid and frees the buffer for more
         * allocations
         */
        void Reset();

        /**
         * @return whether the allocator is in a valid state
         */
        bool IsInitialized() const;

        uintptr_t Begin() const override;
        uintptr_t End() const override;


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
