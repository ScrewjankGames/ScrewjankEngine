module;

// STD Headers
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory_resource>
#include <utility>

export module sj.engine.system.memory.allocators:Allocator;

export namespace sj
{
    /**
     * Concept for template deduction required when supplying allocators to containers
     */
    template <class T>
    concept allocator_concept = requires(T obj) {
        // These statements must compile for something to be considered an allocator
        { obj.allocate(size_t()) };
        { obj.deallocate(nullptr) };
    };

    /**
     * Concept for template deduction required when supplying allocators to containers
     */
    template <class T>
    concept is_allocator_ptr = requires(T obj) {
        // These statements must compile for something to be considered an allocator pointer
        { obj->allocate(size_t()) };
        { obj->deallocate(nullptr) };
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

    class memory_resource : std::pmr::memory_resource
    {
    public:
        // Derived resources may provide constructors similar to these, but all resources should be
        // initializable post-construction
        virtual void init(std::pmr::memory_resource& hostResource, size_t numBytes) = 0;
        virtual void init(void* buffer, size_t numBytes) = 0;

        virtual bool contains_ptr(void* ptr) const = 0;

#ifndef GOLD_VERSION
        void set_debug_name(const char* name)
        {
            strncpy(m_DebugName, name, sizeof(m_DebugName));
        }

    private:
        char m_DebugName[256];
#endif
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
        [[nodiscard]]
        virtual void* allocate(const size_t size,
                               const size_t alignment = alignof(std::max_align_t)) = 0;

        /**
         * Marks memory as free
         * @param memory Pointer to the memory to free
         */
        virtual void deallocate(void* memory) = 0;

        /**
         * Allocate enough aligned memory for the provided type
         * @tparam T the type to allocate memory for
         */
        template <class T>
        [[nodiscard]] void* AllocateType()
        {
            return allocate(sizeof(T), alignof(T));
        }

        virtual uintptr_t Begin() const = 0;
        virtual uintptr_t End() const = 0;
    };

} // namespace sj
