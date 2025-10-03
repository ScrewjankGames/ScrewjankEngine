module;

#include <concepts>
#include <ranges>

export module sj.std.concepts;

export namespace sj
{
template <class Rg, class Elem>
concept range_of = std::ranges::range<Rg> && std::same_as<Elem, std::ranges::range_value_t<Rg>>;

}