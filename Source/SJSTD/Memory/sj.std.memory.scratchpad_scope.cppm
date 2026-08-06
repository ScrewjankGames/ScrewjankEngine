module;

#include <cstddef>

export module sj.std.memory.scratchpad_scope;
import sj.std.memory.resources.linear_allocator;
import sj.std.memory.resources;

export namespace sj
{
class scratchpad_scope : public sj::memory_resource
{
public:
    scratchpad_scope(linear_allocator& resource) : m_resource(resource)
    {
        m_originalOffset = m_resource.get_current_offset();
    }

    ~scratchpad_scope()
    {
        m_resource.reset(m_originalOffset);
    }

    [[nodiscard]]
    void* do_allocate(const size_t size,
                      const size_t alignment = alignof(std::max_align_t)) override
    {
        return m_resource.allocate(size, alignment);
    }

    void do_deallocate(void* memory, size_t bytes, size_t alignment) override
    {
        m_resource.deallocate(memory, bytes, alignment);
    }

    [[nodiscard]]
    bool contains_ptr(void* ptr) const override
    {
        return m_resource.contains_ptr(ptr);
    }

private:
    size_t m_originalOffset = 0;
    linear_allocator& m_resource;
};
} // namespace sj