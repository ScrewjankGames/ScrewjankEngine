module;

#include <stack>

export module sj.std.containers.stack;
import sj.std.containers.vector;

export namespace sj
{
    template<class T, size_t tCapacity>
    using static_stack = std::stack<T, static_vector<T, tCapacity>>;
}