module;

#include <cstddef>
#include <functional>

#if __cpp_lib_flat_set >= 202207L
#include <flat_set>
#else
#include <SG14/flat_set.h>
#endif

export module sj.std.containers:set;
import :vector;

#if __cpp_lib_flat_set >= 202207L
#define set_type std::flat_set
#else
#define set_type stdext::flat_set
#endif

export namespace sj
{
    template<class Key, size_t tN, class Compare = std::less<Key>>
    using static_set = set_type<Key, Compare, static_vector<Key,tN>>;
}