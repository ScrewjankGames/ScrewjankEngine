module;

#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/Log.hpp>
#include <memory_resource>

export module sj.std.memory.allocators:SystemAllocator;
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
                          [[maybe_unused]] const size_t alignment = alignof(std::max_align_t)) override
        {
            return std::malloc(size); // NOLINT
        }

        void do_deallocate(void* p, [[maybe_unused]] size_t bytes, [[maybe_unused]] size_t alignment) override
        {
            std::free(p); // NOLINT
        }
    };
} // namespace sj