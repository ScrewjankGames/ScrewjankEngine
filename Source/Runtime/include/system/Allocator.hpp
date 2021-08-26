#pragma once
// STD Headers
#include <cstddef>
#include <utility>

namespace sj {

    /**
     * Concept for template deduction required when supplying allocators to containers
     */
    template <class T>
    concept AllocatorConcept = requires(T obj)
    {
        // These statements must compile for something to be considered an allocator
        {obj.Allocate(size_t())};
        {obj.Free(nullptr)};
    };

    /**
     * Concept for template deduction required when supplying allocators to containers
     */
    template <class T>
    concept AllocatorPtrConcept = requires(T obj)
    {
        // These statements must compile for something to be considered an allocator pointer
        {obj->Allocate(size_t())};
        {obj->Free(nullptr)};
    };

    struct AllocatorStatus
    {
        size_t Capacity;
        size_t ActiveAllocationCount;
        size_t ActiveBytesAllocated;
        size_t TotalAllocationCount;
        size_t TotalBytesAllocated;

        AllocatorStatus()
        {
            Capacity = 0;
            ActiveAllocationCount = 0;
            ActiveBytesAllocated = 0;
            TotalAllocationCount = 0;
            TotalBytesAllocated = 0;
        }

        size_t FreeSpace()
        {
            return Capacity - ActiveBytesAllocated;
        }
    };

    class Allocator
    {
      public:
        /**
         * Constructor
         */
        Allocator() = default;

        /**
         * Destructor
         */
        virtual ~Allocator() = default;

        /**
         * Allocates size bites from the heap
         * @param size The number of bytes to allocate
         * @param alignment The alignment requirement for this allocation
         */
        [[nodiscard]] virtual void*
        Allocate(const size_t size, const size_t alignment = alignof(std::max_align_t)) = 0;

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
        [[nodiscard]] void* AllocateType();

        /**
         * Helper function that utilizes the allocator to allocate and construct an object
         * @param args... Arguments to be forwarded to the type's constructor
         */
        template <class T, class... Args>
        [[nodiscard]] T* New(Args&&... args);

        /**
         * Helper function that utilizes the allocator to destruct and deallocate an object
         * @param memory Pointer to the object to be deleted
         */
        template <class T>
        void Delete(T*& memory);
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
    inline void Allocator::Delete(T*& memory)
    {
        // Call object destructor
        memory->~T();

        // Deallocate object
        Free(memory);

        // Null out supplied pointer
        memory = nullptr;
    }

} // namespace sj
