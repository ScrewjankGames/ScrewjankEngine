#pragma once
// STD Headers
#include <cstddef>
#include <concepts>
#include <array>

// Screwjank Headers
#include <ScrewjankEngine/core/Assert.hpp>
#include <ScrewjankEngine/system/memory/Memory.hpp>
#include <ScrewjankEngine/utils/Concepts.hpp>

namespace sj 
{
    template <class T, std::size_t N> 
	using array = std::array<T, N>;

} // namespace sj
