module;
#include <ScrewjankStd/Assert.hpp>

#include <beman/inplace_vector/inplace_vector.hpp>

#include <memory_resource>

// End global module fragment
export module sj.std.containers.vector;

export namespace sj
{
template <class T, size_t N>
using static_vector = beman::inplace_vector::inplace_vector<T, N>;

template <class T, class Alloc = std::pmr::polymorphic_allocator<T>>
using dynamic_vector = std::vector<T, Alloc>;

template <class vector_like>
void erase_unordered(vector_like& v, auto&& it)
{
    SJ_ASSERT(it != v.end(), "Invalid element");

    std::iter_swap(it, v.end() - 1);
    v.pop_back();
}
} // namespace sj
