module;

#include <flat_map>

export module sj.std.containers:map;
import :vector;

export namespace sj 
{
    template<class KeyType, class ValueType, size_t tCount>
	using static_flat_map = std::flat_map<KeyType, ValueType, std::less<KeyType>, static_vector<KeyType, tCount>, static_vector<ValueType, tCount>>;

    template<class KeyType, class ValueType>
	using dynamic_flat_map = std::flat_map<KeyType, ValueType, std::less<KeyType>, dynamic_vector<KeyType>, dynamic_vector<ValueType>>;
}
