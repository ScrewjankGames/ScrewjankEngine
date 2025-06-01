module;

#include <cstddef>
#include <flat_set>
#include <functional>

export module sj.std.containers:set;
import :vector;

export namespace sj
{
    template<class Key, size_t tN, class Compare = std::less<Key>>
    using static_set = std::flat_set<Key, Compare, static_vector<Key,tN>>;
}