module;

#include <ScrewjankShared/utils/MemUtils.hpp>
#include <ScrewjankShared/utils/Assert.hpp>
#include <ScrewjankShared/utils/Log.hpp>
#include <memory_resource>

export module sj.engine.system.memory.allocators:SystemAllocator;
import :Allocator;

export namespace sj
{
    class SystemAllocator final : public std::pmr::memory_resource
    {
    public:
        [[nodiscard]] bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override
        {
            return &other == this;
        }

    private:
        [[nodiscard]]
        void* do_allocate(const size_t size,
                          const size_t alignment = alignof(std::max_align_t)) override
        {
            return std::malloc(size);
        }

        void do_deallocate(void* p, size_t bytes, size_t alignment) override
        {
            std::free(p);
        }
    };
} // namespace sj