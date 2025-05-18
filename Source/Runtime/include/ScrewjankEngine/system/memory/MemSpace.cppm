module;

// STD Headers
#include <memory_resource>
#include <mutex>
#include <cstddef>

// Screwjank Engine Headers
#include <ScrewjankShared/utils/Assert.hpp>

export module sj.engine.system.memory:MemSpace;
import sj.engine.system.memory.allocators;
import sj.shared.containers;

export namespace sj
{
    /**
     * Represents a named zone of memory on the heap, owns associated memory or defers to parent
     * MemSpace
     */
    class IMemSpace : public std::pmr::memory_resource
    {
    public:
        virtual ~IMemSpace() = default;

        template <class T>
        [[nodiscard]] T* allocate_type(const size_t count = 1)
        {
            return static_cast<T*>(allocate(sizeof(T) * count, alignof(T)));
        }

        template <class T, class... Args>
        [[nodiscard]] T* sj_new(Args&&... args)
        {
            return new(this->allocate_type<T>()) T(std::forward<Args>(args)...);
        }

        /**
         * Helper function that utilizes the allocator to destruct and deallocate an object
         * @param memory Pointer to the object to be deleted
         */
        template <class T>
        void sj_delete(T* memory)
        {
            memory->~T();
            this->deallocate(memory, sizeof(T), alignof(T));
            memory = nullptr;
        }

        virtual bool contains_ptr(void* ptr) const = 0;

        inline static static_vector<IMemSpace*, 64> s_MemSpaceList = {};

    protected:
#ifndef GOLD_VERSION
        char m_DebugName[256];
#endif
    };

    IMemSpace* find_mem_space(void* ptr)
    {
        for(IMemSpace* zone : IMemSpace::s_MemSpaceList)
        {
            if(zone && zone->contains_ptr(ptr))
            {
                return zone;
            }
        }

        return nullptr;
    }
    /**
     * Specialized heap zone that accepts an allocator type
     */
    template <allocator_concept AllocatorType = FreeListAllocator>
    class MemSpace final : public IMemSpace
    {
    public:
        MemSpace() = default;

        MemSpace(IMemSpace* parent, const size_t size, const char* debug_name = "")
        {
            init(parent, size, debug_name);
        }

        ~MemSpace() override
        {
            s_MemSpaceList.erase_element(this);

            if(m_ParentZone)
            {
                m_ParentZone->deallocate(reinterpret_cast<void*>(m_Allocator.Begin()), m_Allocator.End() - m_Allocator.Begin());
            }
            else
            {
                free(reinterpret_cast<void*>(m_Allocator.Begin()));
            }
        }

        void init(IMemSpace* parent, const size_t size, const char* debug_name = "")
        {
            m_ParentZone = parent;

            s_MemSpaceList.push_back(this);

            void* start;

            if(m_ParentZone)
            {
                start = parent->allocate(size);
            }
            else if(size > 0)
            {
                start = malloc(size);
            }

            m_Allocator.Init(size, start);

#ifndef SJ_GOLD
            strncpy(m_DebugName, debug_name, sizeof(m_DebugName));
#endif // !SJ_GOLD
        }

        bool contains_ptr(void* ptr) const override
        {
            uintptr_t ptr_int = reinterpret_cast<uintptr_t>(ptr);

            return ptr_int >= m_Allocator.Begin() && ptr_int < m_Allocator.End();
        }

    private:
        void* do_allocate(size_t bytes, size_t alignment) override
        {
            return m_Allocator.allocate(bytes, alignment);
        };

        void do_deallocate(void* memory, size_t bytes, size_t alignment) override
        {
            m_Allocator.deallocate(memory);
        };

        bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override
        {
            return this == &other;
        }

        IMemSpace* m_ParentZone = nullptr;
        AllocatorType m_Allocator;

        // TODO: Actually put together a thread friendly memory model and remove me
        std::mutex m_HeapLock;
    };

    /**
     * Memspace for allocations we can't be bothered to track right now
     */
    using UnmanagedMemSpace = MemSpace<SystemAllocator>;

    MemSpace() -> MemSpace<FreeListAllocator>;

} // namespace sj
