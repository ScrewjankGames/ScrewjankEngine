module;
#include <cmath>
#include <array>

export module sj.std.math:Vec2;

export namespace sj
{
class vec2
{
public:
    constexpr vec2() = default;

    constexpr vec2(float x, float y) : m_elements {x, y}
    {
    }

    [[nodiscard]] constexpr float get_x() const
    {
        return m_elements[0];
    }

    [[nodiscard]] constexpr float get_y() const
    {
        return m_elements[1];
    }

    constexpr vec2& set_x(float x)
    {
        m_elements[0] = x;
        return *this;
    }

    constexpr vec2& set_y(float y)
    {
        m_elements[1] = y;
        return *this;
    }

    constexpr inline bool operator==(const vec2& other) const
    {
        return m_elements[0] == other.m_elements[0] && m_elements[1] == other.m_elements[1];
    }

private:
    std::array<float, 2> m_elements;
};

inline constexpr vec2 Vec2_Zero = vec2(0.0f, 0.0f);
inline constexpr vec2 Vec2_UnitX = vec2(1.0f, 0.0f);
inline constexpr vec2 Vec2_UnitY = vec2(0.0f, 0.1f);

[[nodiscard]] constexpr vec2 operator*(const vec2& v, const float s)
{
    return vec2(v.get_x() * s, v.get_y() * s);
}

[[nodiscard]] constexpr vec2 operator*(const float s, const vec2& v)
{
    return v * s;
}

[[nodiscard]] constexpr vec2 operator/(const vec2& v, const float s)
{
    return vec2(v.get_x() / s, v.get_y() / s);
}

[[nodiscard]] constexpr vec2 operator+(const vec2& a, const vec2& b)
{
    return vec2(a.get_x() + b.get_x(), a.get_y() + b.get_y());
}

[[nodiscard]] constexpr float magnitude_sqr(const vec2& v)
{
    return (v.get_x() * v.get_x()) + (v.get_y() * v.get_y());
}

[[nodiscard]] constexpr float magnitude(const vec2& v)
{
    return std::sqrtf(magnitude_sqr(v));
}

[[nodiscard]] constexpr vec2 normalized(const vec2& v)
{
    return v / magnitude(v);
}
} // namespace sj
