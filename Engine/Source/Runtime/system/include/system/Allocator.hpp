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
        Allocator(const char* debug_name = "");

        /** Destructor */
        virtual ~Allocator() = default;
        /**
         * Allocates size bites from the heap
         * @param size The number of bytes to allocate
         */
        virtual void* Allocate(const size_t size, const size_t alignment = 1) = 0;

        /**
         * Marks memory as free
         * @param memory Pointer to the memory to free
         */
        virtual void Free(void* memory = nullptr) = 0;

        template <class T>
        void* AllocateType();

        /**
         * Helper function to allocate and construct object using any allocator
         */
        template <class T, class... Args>
        T* New(Args&&... args);

        /**
         * Helper function to deallocate and deconstruct object using any allocator
         */
        template <class T>
        void Delete(T*& ptr);

      protected:
        const char* m_DebugName;
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

    template <class T>
    void* Allocator::AllocateType()
    {
        return Allocate(sizeof(T), alignof(T));
    }

    template <class T, class... Args>
    inline T* Allocator::New(Args&&... args)
    {
        return new (AllocateType<T>()) T(std::forward<Args>(args)...);
    }

    template <class T>
    inline void Allocator::Delete(T*& ptr)
    {
        // Call object destructor
        ptr->~T();

        // Deallocate object
        Free(ptr);

        // Null out supplied pointer
        ptr = nullptr;
    }

} // namespace Screwjank
