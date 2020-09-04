// STD Headers

// Library headers

// Engine Headers

namespace Screwjank {
    class Allocator
    {
      public:
        /**
         * Constructor
         * @param buffer_size
         */
        Allocator(size_t buffer_size);

        /**
         * Allocates size bites from the heap
         * @param size The number of bytes to allocate
         */
        virtual void* Allocate(size_t size) = 0;

        /**
         * Marks memory as free
         * @param memory Pointer to the memory to free
         */
        virtual void Free(void* memory) = 0;
    };

    /**
     * @class BasicAllocator
     * @brief A simple wrapper for standard malloc and free calls
     */
    class BasicAllocator : public Allocator
    {
      public:
        BasicAllocator() = default;
        void* Allocate(size_t size) override;
        void Free(void* memory) override;
    };

    template <size_t block_size>
    class PoolAllocator : public Allocator
    {
      public:
        PoolAllocator(size_t buffer_size);
    };
} // namespace Screwjank
