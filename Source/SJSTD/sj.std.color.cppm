module;

export module sj.std.color;
import sj.std.primitives;

export namespace sj
{
struct color
{
    float r;
    float g;
    float b;
    float a;
};

namespace colors
{
    
inline constexpr color red {.r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f};
inline constexpr color green {.r = 0.0f, .g = 1.0f, .b = 0.0f, .a = 1.0f};
inline constexpr color blue {.r = 0.0f, .g = 0.0f, .b = 1.0f, .a = 1.0f};

} // namespace colors
} // namespace sj