module;

#include <tuple>
#include <utility>

export module sj.std.tuple;

export namespace sj
{
template <class... Ts, class Fn>
constexpr void for_each_reverse( std::tuple<Ts...>& t, Fn&& f )
{
    constexpr std::size_t kNumTypes = std::tuple_size_v<std::tuple<Ts...>>;

    [&]<auto... Is>(std::index_sequence<Is...>) {
        (f.template operator()(std::get<kNumTypes - Is - 1>(t)), ...);
    }(std::index_sequence_for<Ts...> {});
}
} // namespace sj