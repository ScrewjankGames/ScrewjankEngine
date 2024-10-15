#pragma once

// Screwjank Headers
#include <ScrewjankEngine/containers/Vector.hpp>

// Library Headers
#include <SG14/flat_map.h>


namespace sj
{
	template<class KeyType, class ValueType, size_t tCount>
	using static_unordered_map = stdext::flat_map<KeyType, ValueType, std::less<KeyType>, static_vector<KeyType, tCount>, static_vector<ValueType, tCount>>;
}