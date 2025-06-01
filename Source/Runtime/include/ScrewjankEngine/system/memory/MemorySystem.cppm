module;

#include <ScrewjankStd/Assert.hpp>

#include <cstdint>
#include <memory_resource>

export module sj.engine.system.memory:MemorySystem;
import :Literals;

import sj.engine.system.memory.allocators;
import sj.engine.system.memory.utils;
import sj.std.containers;

export namespace sj
{
    // Root heap sizes
    constexpr uint64_t kRootHeapSize = 5_MiB;
    constexpr uint64_t kDebugHeapSize = 64_MiB;

    class MemorySystem
    {
    public:
        static void Init()
        {
            // The first thing the program does once entering main should
            // be initializing the memory system, which happens on first access.
            Get();
        }

        /**
         * Provides global access to the memory system
         */
        static MemorySystem* Get()
        {
            static MemorySystem memSys;
            return &memSys;
        }

        static FreeListAllocator* GetRootMemoryResource()
        {
            return &(Get()->m_rootResource);
        }

        static std::pmr::memory_resource* GetUnmanagedMemoryResource()
        {
            return &(Get()->m_unmanagedResource);
        }

#ifndef SJ_GOLD
        static FreeListAllocator* GetDebugMemoryResource()
        {
            return &(Get()->m_debugResource);
        }
#endif

        static void TrackMemoryResource(sj::memory_resource* resource)
        {
            Get()->s_trackedResources.emplace_back(resource);
        }

        static void PushMemoryResource(std::pmr::memory_resource* mem_resource)
        {
            s_memoryResourceStack.push(mem_resource);
        }

        static void PopMemoryResource()
        {
            s_memoryResourceStack.pop();
        }

        /**
         * Users can push and pop memory resources off the stack to control allocations for scopes
         * Allows custom allocators to be used with third party libraries.
         */
        [[nodiscard]]
        static std::pmr::memory_resource* GetCurrentMemoryResource()
        {
            if(s_memoryResourceStack.empty())
            {
                return GetUnmanagedMemoryResource();
            }
            else
            {
                return s_memoryResourceStack.top();
            }
        }

        [[nodiscard]]
        static sj::memory_resource* FindOwningResource(void* ptr)
        {
            for(sj::memory_resource* resource : s_trackedResources)
            {
                if(resource->contains_ptr(ptr))
                    return resource;
            }

            return nullptr;
        }

    private:
        FreeListAllocator m_rootResource;

#ifndef SJ_GOLD
        FreeListAllocator m_debugResource;
#endif

        SystemAllocator m_unmanagedResource;

        MemorySystem() : m_rootResource(kRootHeapSize, std::pmr::new_delete_resource())
        {
            s_trackedResources.emplace_back(&m_rootResource);

#ifndef SJ_GOLD
            m_rootResource.set_debug_name("Root Heap");

            m_debugResource.init(kDebugHeapSize, std::pmr::new_delete_resource());
            m_debugResource.set_debug_name("Debug Heap");
            s_trackedResources.emplace_back(&m_debugResource);
#endif
        }

        ~MemorySystem() = default;

        inline static thread_local static_stack<std::pmr::memory_resource*, 64>
            s_memoryResourceStack = {};

        // Can be searched to figure out where a pointer came from
        inline static static_vector<sj::memory_resource*, 64> s_trackedResources = {};
    };

    /**
     * Helper class that pushes a memory resource onto the stack on create
     * and pops it when it goes out of scope
     */
    class MemoryResourceScope
    {
    public:
        MemoryResourceScope(std::pmr::memory_resource* resource)
            : m_resource(resource)
        {
            MemorySystem::PushMemoryResource(resource);
        }
        MemoryResourceScope(const MemoryResourceScope& other) = delete;
        MemoryResourceScope(MemoryResourceScope&& other) noexcept : MemoryResourceScope(other.m_resource)
        {
            other.m_resource = nullptr;
        }

        MemoryResourceScope& operator=(const MemoryResourceScope& other) = delete;

        ~MemoryResourceScope()
        {
            MemorySystem::PopMemoryResource();
        }

        std::pmr::memory_resource& operator*()
        {
            return *m_resource;
        }

        std::pmr::memory_resource* operator->()
        {
            return m_resource;
        }
        const std::pmr::memory_resource& operator*() const
        {
            return *m_resource;
        }
        const std::pmr::memory_resource* operator->() const
        {
            return m_resource;
        }

        std::pmr::memory_resource* Get()
        {
            return m_resource;
        }

    private:
        std::pmr::memory_resource* m_resource;
    };

} // namespace sj
