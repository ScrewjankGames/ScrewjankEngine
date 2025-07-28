module;

#include <cstddef>

export module sj.engine.system.threading.ThreadContext;
export import sj.std.memory.scratchpad_scope;
import sj.std.memory.resources;

export namespace sj
{
    class ThreadContext
    {
    public:
        static void Init(sj::memory_resource* backing_resource, size_t scratchpadSize)
        {
            s_scratchpadParentResource = backing_resource;
            void* memory = backing_resource->allocate(scratchpadSize);
            s_scratchpadAllocator.init(scratchpadSize, reinterpret_cast<std::byte*>(memory));
        }

        static void DeInit()
        {
            s_scratchpadParentResource->deallocate(s_scratchpadAllocator.data(), s_scratchpadAllocator.buffer_size());
        }

        [[nodiscard]] static scratchpad_scope GetScratchpad()
        {
            return scratchpad_scope(s_scratchpadAllocator);
        }

    private:
        static inline thread_local sj::memory_resource* s_scratchpadParentResource = nullptr;
        static inline thread_local linear_allocator s_scratchpadAllocator = {};
    };

}; // namespace sj
