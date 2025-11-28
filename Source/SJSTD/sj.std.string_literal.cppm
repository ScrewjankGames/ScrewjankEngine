module;
#include <array>
#include <ranges>
export module sj.std.string_literal;

export namespace sj
{
// Wrapper to use string literals as non-type template parameters
template <size_t N>
struct string_literal
{
    constexpr string_literal(const char (&str)[N])
    {
        std::ranges::copy(str, value.begin());
    }

    std::array<char, N> value;
};

} // namespace sj