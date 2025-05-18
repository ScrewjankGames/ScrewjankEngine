module;

#include <ScrewjankShared/utils/MemUtils.hpp>
#include <ScrewjankShared/utils/Assert.hpp>

#include <memory>
#include <cstdint>
#include <cstddef>
#include <new>

export module sj.engine.system.memory:MemorySystem;
import :MemSpace;
import :Literals;
import sj.engine.system.memory.allocators;
import sj.shared.containers;

export namespace sj
{
    extern thread_local static_stack<IMemSpace*, 64> g_MemSpaceStack;

    // Root heap sizes
    constexpr uint64_t kRootHeapSize = 5_MiB;
    constexpr uint64_t kDebugHeapSize = 64_MiB;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// Memory Manager
    ///////////////////////////////////////////////////////////////////////////////////////////////
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

        static void PushMemSpace(IMemSpace* mem_space)
        {
            g_MemSpaceStack.push(mem_space);
        }
        static void PopMemSpace()
        {
            g_MemSpaceStack.pop();
        }

        static IMemSpace* GetRootMemSpace()
        {
            return &(Get()->m_RootMemSpace);
        }

        static UnmanagedMemSpace* GetUnmanagedMemSpace()
        {
            return &(Get()->m_UnmanagedMemSpace);
        }

#ifndef SJ_GOLD
        static IMemSpace* GetDebugMemSpace()
        {
            return &(Get()->m_DebugMemSpace);
        }
#endif

        /**
         * Users can push and pop heap zones off the stack to control allocations for scopes
         * Allows custom allocators to be used with third party libraries.
         */
        [[nodiscard]]
        static IMemSpace* GetCurrentMemSpace()
        {
            MemorySystem* system = Get();

            if(g_MemSpaceStack.empty())
            {
                return GetUnmanagedMemSpace();
            }
            else
            {
                return g_MemSpaceStack.top();
            }
        }

    private:
        SystemAllocator m_unmanagedResource;
#ifndef SJ_GOLD
        FreeListAllocator m_debugResource;
#endif
        FreeListAllocator m_rootResource;

        UnmanagedMemSpace m_UnmanagedMemSpace;
#ifndef SJ_GOLD
        MemSpace<FreeListAllocator> m_DebugMemSpace;
#endif
        MemSpace<FreeListAllocator> m_RootMemSpace;

        MemorySystem()

            : m_RootMemSpace(nullptr, kRootHeapSize, "Root Heap"),
              m_UnmanagedMemSpace(nullptr, 0, "Unmanaged Allocations")
#ifndef SJ_GOLD
              ,
              m_DebugMemSpace(nullptr, kDebugHeapSize, "Debug Heap")
#endif // !SJ_GOLD
        {
            // s_trackedResources.emplace_back(&m_rootResource);

            // #ifndef SJ_GOLD
            // s_trackedResources.emplace_back(&m_debugResource);
            // #endif
        }

        ~MemorySystem() = default;

        inline static static_vector<sj::memory_resource*, 64> s_trackedResources = { };

        inline static thread_local static_stack<IMemSpace*, 64> g_MemSpaceStack = {};
    };

    /**
     * Helper class that pushes a heap zone onto the stack on create
     * and pops it when it goes out of scope
     */
    class MemSpaceScope
    {
    public:
        MemSpaceScope(IMemSpace* zone)
        {
            MemorySystem::PushMemSpace(zone);
        }
        MemSpaceScope(const MemSpaceScope& other) = delete;
        MemSpaceScope(MemSpaceScope&& other) : MemSpaceScope(other.m_Heap)
        {
            other.m_Heap = nullptr;
        }

        MemSpaceScope& operator=(const MemSpaceScope& other) = delete;

        ~MemSpaceScope()
        {
            MemorySystem::PopMemSpace();
        }

        IMemSpace& operator*()
        {
            return *m_Heap;
        }
        IMemSpace* operator->()
        {
            return m_Heap;
        }
        const IMemSpace& operator*() const
        {
            return *m_Heap;
        }
        const IMemSpace* operator->() const
        {
            return m_Heap;
        }

        IMemSpace* Get()
        {
            return m_Heap;
        }

    private:
        IMemSpace* m_Heap;
    };

} // namespace sj
