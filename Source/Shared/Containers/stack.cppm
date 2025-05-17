module;

#include <stack>

export module sj.shared.containers:stack;
import :vector;

export namespace sj
{
    template<class T, size_t tCapacity>
    using static_stack = std::stack<T, static_vector<T, tCapacity>>;
}