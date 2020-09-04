#pragma once
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
        Allocator() = default;

        /** Destructor */
        virtual ~Allocator() = default;
        /**
         * Allocates size bites from the heap
         * @param size The number of bytes to allocate
         */
        virtual void* Allocate(const size_t size, const size_t alignment = 0) = 0;

        /**
         * Marks memory as free
         * @param memory Pointer to the memory to free
         */
        virtual void Free(void* memory = nullptr) = 0;

        virtual void Log();
    };

    /**
     * @class BasicAllocator
     * @brief A simple wrapper for standard malloc and free calls
     */
    class BasicAllocator : public Allocator
    {
      public:
        BasicAllocator() = default;
        void* Allocate(const size_t size, const size_t alignment = 0) override;
        void Free(void* memory) override;
    };

} // namespace Screwjank
