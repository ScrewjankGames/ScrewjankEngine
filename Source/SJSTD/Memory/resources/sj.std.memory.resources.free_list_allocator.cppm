module;

#include <ScrewjankStd/Assert.hpp>

#include <algorithm>
#include <utility>
#include <cstdint>

export module sj.std.memory.resources.free_list_allocator;
import sj.std.memory.resources.memory_resource;
import sj.std.memory.utils;
import sj.std.containers.unmanaged_list;

export namespace sj
{
    class free_list_allocator final : public sj::memory_resource
    {
    public:
        /**
         * Constructor
         */
        free_list_allocator() : m_bufferStart(nullptr), m_bufferEnd(nullptr)
        {
        }

        /**
         * Initializing Constructor
         */
        free_list_allocator(size_t numBytes, std::pmr::memory_resource& hostResource)
            : free_list_allocator()
        {
            sj::memory_resource::init(numBytes, hostResource);
        }

        /**
         * Initializing Constructor
         */
        free_list_allocator(size_t buffer_size, std::byte* memory) : free_list_allocator()
        {
            init(buffer_size, memory);
        }

        /**
         * Copy Constructor
         */
        free_list_allocator(const free_list_allocator& other) = delete;

        /**
         * Move Constructor
         */
        free_list_allocator(free_list_allocator&& other) = delete;

        /**
         * Destructor
         */
        ~free_list_allocator() final = default;

        /**
         * @return whether the allocator is in a valid state
         */
        [[nodiscard]] bool is_initialized() const
        {
            return m_bufferStart != nullptr;
        }

        using sj::memory_resource::init;

        void init(size_t buffer_size, std::byte* memory) override
        {
            SJ_ASSERT(!is_initialized(), "Double initialization of free list allocator detected");

            SJ_ASSERT(buffer_size > sizeof(FreeBlock) && buffer_size > sizeof(AllocationHeader),
                      "free_list_allocator is not large enough to hold data");

            // Allocate memory from backing allocator
            m_bufferStart = memory;
            m_bufferEnd = reinterpret_cast<void*>((uintptr_t(memory) + buffer_size));

            // Initialize free list to be a single block of buffer_size situated at start of buffer
            auto* initial_block = new(m_bufferStart) FreeBlock(buffer_size);
            AddFreeBlock(initial_block);
        }

        bool contains_ptr(void* memory) const override
        {
            return IsPointerInAddressSpace(memory, m_bufferStart, m_bufferEnd);
        }

    private:
        /**
         * Allocates size bites with given alignment in a best-fit manner
         * @param size The number of bytes to allocate
         */
        [[nodiscard]]
        void* do_allocate(const size_t size,
                          const size_t alignment = alignof(std::max_align_t)) override
        {
            SJ_ASSERT(m_freeBlocks.back().next == nullptr, "what");

            SJ_ASSERT(is_initialized(), "Trying to allocate with uninitialized allocator");

            // Search the free list, and return the most suitable free block and the padding
            // required to use it
            std::pair<FreeBlock*, uint32_t> free_list_search_result =
                FindFreeBlock(size, alignment);

            FreeBlock* const best_fit_block = free_list_search_result.first;
            const uint32_t header_padding = free_list_search_result.second;

            // If no best fit block was found, halt program
            SJ_ASSERT(best_fit_block != nullptr, "Free list allocator is out of memory.");

            // Remove block from the free list
            m_freeBlocks.erase(best_fit_block);
            if(!m_freeBlocks.empty())
                SJ_ASSERT(m_freeBlocks.back().next == nullptr, "what");

            // Get a copy of the current block's info before it is overwritten
            FreeBlock old_block_info = *best_fit_block;

            // Calculate allocation information
            void* const header_address =
                reinterpret_cast<void*>(uintptr_t(best_fit_block) + header_padding);
            void* const payload_address =
                reinterpret_cast<void*>(uintptr_t(header_address) + sizeof(AllocationHeader));

            // Amount of space left in the block for the payload
            // !!!This operation overwrites the data pointed to by best_fit_block!!!
            auto payload_space = old_block_info.size - sizeof(AllocationHeader) - header_padding;

            SJ_ASSERT(IsMemoryAligned(header_address, alignof(AllocationHeader)),
                      "Allocation header memory is misaligned");

            SJ_ASSERT(IsMemoryAligned(payload_address, alignment),
                      "Allocation payload memory is misaligned");

            // Place the header into memory just before the user's data
            auto header = new(header_address) AllocationHeader(header_padding, payload_space);

            // Calculate unused space and attempt to make a new free block
            uintptr_t payload_end = uintptr_t(payload_address) + size;
            uintptr_t block_end = uintptr_t(payload_address) + payload_space;

            // Padding required to align a new free block after the end of the user payload
            auto new_block_adjustment =
                GetAlignmentAdjustment(alignof(FreeBlock), reinterpret_cast<void*>(payload_end));

            // Padding bytes for the new block would be left at the end of the current block
            auto unused_space = block_end - (payload_end + new_block_adjustment);

            if(unused_space >= kMinBlockSize)
            {
                // Remove unused_space from the allocation header's representation of the block
                header->size -= unused_space;
                SJ_ASSERT(header->size > 0, "Shit");
                // Place a FreeBlock into the buffer after the user's payload
                void* new_block_address =
                    reinterpret_cast<void*>(payload_end + new_block_adjustment);
                FreeBlock* new_block = new(new_block_address) FreeBlock(unused_space);

                // Insert the new block into the free list
                AddFreeBlock(new_block);
            }


            return payload_address;
        }

        /**
         * Marks memory as free
         * @param memory Pointer to the memory to free
         */
        void do_deallocate(void* memory,
                           [[maybe_unused]] size_t bytes,
                           [[maybe_unused]] size_t alignment) override
        {
            SJ_ASSERT(is_initialized(), "Trying to deallocate with uninitialized allocator");
            SJ_ASSERT(memory != nullptr, "Cannot free nullptr");
            SJ_ASSERT(contains_ptr(memory), "Pointer is not managed by this allocator!");

            AllocationHeader* block_header = GetAllocationHeader(memory);

            // Extract header info
            auto block_size = block_header->padding + block_header->size + sizeof(AllocationHeader);
            void* block_start =
                reinterpret_cast<void*>(uintptr_t(block_header) - block_header->padding);

            SJ_ASSERT(IsMemoryAligned(block_start, alignof(FreeBlock)),
                      "Free block is mis-aligned");

            FreeBlock* new_block = new(block_start) FreeBlock(block_size);

            AddFreeBlock(new_block);
            SJ_ASSERT(m_freeBlocks.back().next == nullptr, "what");
        }

        /** Linked list node structure inserted in-place into the allocator's buffer */
        struct FreeBlock
        {
            size_t size = 0;
            FreeBlock* prev = nullptr;
            FreeBlock* next = nullptr;

            [[nodiscard]] FreeBlock* get_next() const
            {
                return next;
            }

            void set_next(FreeBlock* block)
            {
                next = block;
            }

            [[nodiscard]] FreeBlock* get_prev() const
            {
                return prev;
            }

            void set_prev(FreeBlock* block)
            {
                prev = block;
            }
        };

        /** Book-keeping structure to correctly de-allocate memory */
        struct AllocationHeader
        {
            /** The padding placed before this header in the free block during allocation */
            uint8_t padding;
            size_t size;
        };

        /** Constant value to determine the minimum size of a block */
        static constexpr size_t kMinBlockSize =
            std::max(sizeof(FreeBlock), sizeof(AllocationHeader) + 1);

        /** Constant value to determine the strictest alignment that must be satisfied to split off
         * a new free block */
        static constexpr size_t kMaxMetadataAlignment =
            std::max(alignof(FreeBlock), alignof(AllocationHeader));

        /**
         * Searches the free list for the first block that can satisfy the size and alignment
         * requirements provided, in addition to the allocation header.
         * @param size The size the user wishes to allocate
         * @param the alignment restriction for the memory the user is requesting
         * @return The most suitable free block, along with the amount of padding that needs to be
         * placed before the allocation header to satisfy the allocation in that block
         */
        [[nodiscard]] std::pair<FreeBlock*, uint32_t> FindFreeBlock(const size_t size,
                                                                    const size_t alignment)
        {
            // TODO (MrLever): Replace linear search with a Red-Black tree

            const auto header_and_payload_size = size + sizeof(AllocationHeader);

            // When looking to align memory, align by the strictest alignment requirement. This will
            // allow header and payload to be packed together in memory without padding between
            // them, only padding in the allocation is placed before the header.
            const size_t alignment_requirement = std::max(alignof(AllocationHeader), alignment);

            for(FreeBlock& curr_block : m_freeBlocks)
            {
                // When allocating, we must calculate alignment from this address to leave room for
                // the allocation header
                void* fist_possible_payload_address =
                    reinterpret_cast<void*>(uintptr_t(&curr_block) + sizeof(AllocationHeader));

                // Get the padding needed to align payload from first possible playload addresss
                size_t required_padding =
                    GetAlignmentAdjustment(alignment_requirement, fist_possible_payload_address);
                size_t total_allocation_size = header_and_payload_size + required_padding;

                // If the current free block is large enough to support allocation
                if(total_allocation_size <= curr_block.size)
                {
                    return {&curr_block, required_padding};
                }
            }

            return {nullptr, 0};
        }

        /**
         * Adds a free block to the free list
         */
        void AddFreeBlock(FreeBlock* new_block)
        {
            auto freeListSearchPred = [new_block]( const FreeBlock& block ) -> bool
            {
                return reinterpret_cast<uintptr_t>(new_block) < reinterpret_cast<uintptr_t>(&block);
            };

            auto insert_pos = std::ranges::find_if(m_freeBlocks, freeListSearchPred);
            m_freeBlocks.insert(insert_pos, new_block);
            
            AttemptCoalesceBlock(new_block);
        }

        /**
         * Given a block in the free list, attempt to merge it with it's neighbors
         */
        void AttemptCoalesceBlock(FreeBlock* curr_block)
        {
            size_t block_size = curr_block->size;

            // Attempt to coalesce with left neighbor, moving block pointer back if necessary
            FreeBlock* prev = curr_block->prev;
            if(prev != nullptr)
            {
                uintptr_t left_end = uintptr_t(prev) + prev->size;

                // If this block starts exactly where the previous block ends, coalesce
                if(uintptr_t(curr_block) == left_end)
                {
                    m_freeBlocks.erase(curr_block);
                    prev->size += block_size;
                    
                    // update curr_block so we can potentially merge again, as a treat
                    curr_block = prev;
                    block_size = prev->size;
                }
            }


            // Attempt to coalesce with right neighbor
            FreeBlock* next = curr_block->next;
            if(next != nullptr)
            {
                uintptr_t block_end = uintptr_t(curr_block) + block_size;

                // If the end of this block is the start of the next block, coalesce
                if(block_end == uintptr_t(next))
                {
                    size_t next_size = next->size;
                    m_freeBlocks.erase(next);
                    curr_block->size += next_size;
                }
            }
        }

        inline AllocationHeader* GetAllocationHeader(void* ptr)
        {
            AllocationHeader* blockHeader =
                reinterpret_cast<AllocationHeader*>(uintptr_t(ptr) - sizeof(AllocationHeader));

            return blockHeader;
        }

        /** The free list of allocation blocks */
        unmanaged_list<FreeBlock> m_freeBlocks;

        /** Pointer to the start of the allocator's memory block */
        void* m_bufferStart;

        /** Pointer to the end of the allocator's memory block */
        void* m_bufferEnd;
    };
} // namespace sj
