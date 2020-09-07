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
         * @param backing_allocator Allocator from which this allocator should request it's memory
         * @param debug_name Name of the allocator used for debugging purposes
         */
        Allocator(Allocator* backing_allocator = nullptr, const char* debug_name = "");

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

        /**
         * Allocate enough aligned memory for the provided type
         * @tparam T the type to allocate memory for
         */
        template <class T>
        void* AllocateType();

        /**
         * Helper function to allocate and construct object using any allocator
         * @tparam T The type to allocate and construct
         * @tparam ...Args The constructor arguments to send to the allocated object
         * @return A pointer to the allocated and constructed object
         */
        template <class T, class... Args>
        T* New(Args&&... args);

        /**
         * Helper function to deallocate and deconstruct object using any allocator
         * @tparam T The type of the pointer being supplied
         * @param ptr Pointer to the memory address to free
         * @note ptr will be nulled after deletion.
         */
        template <class T>
        void Delete(T*& ptr);

      protected:
        const char* m_DebugName;
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
