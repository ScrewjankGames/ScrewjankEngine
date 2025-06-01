module;
#include <type_traits>

export module sj.std.type_traits;

export namespace sj 
{
template <class T, template <auto...> class U>
inline constexpr bool is_instantiation_of_v = std::false_type{};

template <template <auto...> class U, auto... Vs>
inline constexpr bool is_instantiation_of_v<U<Vs...>, U> = std::true_type{};
} // namespace sj
