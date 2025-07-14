module;

#include <version>

#if __cpp_lib_flat_map >= 202207L
#include <flat_map>
#else
#include <stdexcept>
#include <SG14/flat_map.h>
#endif
export module sj.std.containers.map;
import sj.std.containers.vector;

#if __cpp_lib_flat_map >= 202207L
#define map_type std::flat_map
#else
#define map_type stdext::flat_map
#endif

export namespace sj 
{
    template<class KeyType, class ValueType, size_t tCount>
	using static_flat_map = map_type<KeyType, ValueType, std::less<KeyType>, static_vector<KeyType, tCount>, static_vector<ValueType, tCount>>;

    template<class KeyType, class ValueType>
	using dynamic_flat_map = map_type<KeyType, ValueType, std::less<KeyType>, dynamic_vector<KeyType>, dynamic_vector<ValueType>>;
}
