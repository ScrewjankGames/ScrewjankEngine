module;

#include <cstddef>

export module sj.std.memory.scratchpad_scope;
import sj.std.memory.resources.linear_allocator;

export namespace sj
{
    class scratchpad_scope
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

        linear_allocator& get_allocator()
        {
            return m_resource;
        }

    private:
        size_t m_originalOffset = 0;
        linear_allocator& m_resource;
    };
} // namespace sj