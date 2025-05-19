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

    class memory_resource : public std::pmr::memory_resource
    {
    public:
        memory_resource() = default;

        // Derived resources may provide constructors similar to these, but all resources should be
        // initializable post-construction
        virtual void init(size_t numBytes, void* buffer) = 0;

        void init(size_t numBytes, std::pmr::memory_resource& hostResource)
        {
            void* memory = hostResource.allocate(numBytes);
            init(numBytes, memory);
        }
        
        [[nodiscard]] virtual bool contains_ptr(void* ptr) const = 0;

#ifndef GOLD_VERSION
        void set_debug_name(const char* name)
        {
            strncpy(m_DebugName, name, sizeof(m_DebugName));
        }
    protected:
        bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override
        {
            return uintptr_t(this) == uintptr_t(&other);
        }

    private:
        char m_DebugName[256];
#endif
    };
} // namespace sj
