module;
#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/Log.hpp>

export module sj.engine.system.memory.allocators:PoolAllocator;
import :Allocator;
import sj.engine.system.memory.utils;
import sj.std.containers;

export namespace sj
{

    /**
     * Allocates memory in fixed size chunks
     * @tparam kBlockSize The size of each chunk in the pool
     */
    template <size_t kBlockSize>
    class PoolAllocator final : public sj::memory_resource
    {
    public:
        /**
         * Default constructor
         */
        PoolAllocator() = default;

        /**
         * Constructor
         * @param block_count The number of blocks of size kBlockSize
         */
        PoolAllocator(size_t buffer_size, void* memory)
        {
            init(buffer_size, memory);
        }

        /**
         * Destructor
         */
        ~PoolAllocator() final = default;

        void init(size_t buffer_size, void* memory) override
        {
            m_BufferStart = memory;
            m_BufferEnd =
                reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_BufferStart) + buffer_size);

            m_NumBlocks = buffer_size / kBlockSize;

            // Ensure block size is large enough to store an allocation header
            static_assert(kBlockSize > sizeof(FreeBlock),
                          "Block size is not large enough to maintain free list");

            SJ_ASSERT(m_NumBlocks > 0, "Cannot create a pool allocator with zero blocks");

            // Create first free block at start of buffer
            m_FreeList.push_front(new(m_BufferStart) FreeBlock());
            FreeBlock* curr_block = &m_FreeList.front();
            auto curr_block_address = uintptr_t(m_BufferStart);

            // Build free list in the buffer
            for(size_t i = 1; i < m_NumBlocks; i++)
            {
                // Calculate the block address of the next block
                curr_block_address += kBlockSize;

                // Create new free-list block
                m_FreeList.insert_after(curr_block, new(reinterpret_cast<void*>(curr_block_address)) FreeBlock());

                // Move the curr block forward
                curr_block = curr_block->Next;
            }
        }

        static constexpr size_t get_block_size()
        {
            return kBlockSize;
        }

        bool contains_ptr(void* memory) const override
        {
            return IsPointerInAddressSpace(memory, m_BufferStart, m_BufferEnd);
        }

    private:
        /** Node structure for the for the free block linked list */
        struct FreeBlock
        {
            FreeBlock* Next = nullptr;

            FreeBlock* get_next()
            {
                return Next;
            }
            
            void set_next(FreeBlock* next)
            {
                Next = next;
            }
        };

        /**
         * Allocates size bites from the heap
         * @param size The number of bytes to allocate
         */
        [[nodiscard]]
        void* do_allocate(const size_t size,
                                  const size_t alignment = alignof(std::max_align_t)) override
        {
            SJ_ASSERT(size <= kBlockSize,
                      "Pool allocator cannot satisfy allocation of size > block size");

            if(m_FreeList.size() == 0)
            {
                SJ_ENGINE_LOG_ERROR("Pool allocator has run out of blocks");
                return nullptr;
            }

            // Take the first available free block
            FreeBlock* free_block = &(m_FreeList.front());
            SJ_ASSERT(IsMemoryAligned(free_block, alignment),
                      "PoolAllocator does not support over-aligned types");

            // Remove the free block from the free list
            m_FreeList.pop_front();

            return reinterpret_cast<void*>(free_block);
        }

        /**
         * Marks memory as free
         * @param memory Pointer to the memory to free
         */
        void do_deallocate(void* memory, [[maybe_unused]] size_t bytes, [[maybe_unused]] size_t alignment) override
        {
            // Ensure memory is in the region managed by this allocator
            SJ_ASSERT(contains_ptr(memory), "Memory is not managed by this allocator");

            // Place a free-list node into the block and push to head of list
            m_FreeList.push_front(new(memory) FreeBlock());
        }

        /** The beginning of this allocator's data buffer */
        void* m_BufferStart = nullptr;

        /** The end of this allocator's data buffer */
        void* m_BufferEnd = nullptr;

        /** Pointer head of singly linked list of free blocks */
        unmanaged_list<FreeBlock> m_FreeList;

        /** Number of blocks managed by this allocator */
        size_t m_NumBlocks = 0;
    };

    template <class T>
    using ObjectPoolAllocator = PoolAllocator<sizeof(T)>;

} // namespace sj
