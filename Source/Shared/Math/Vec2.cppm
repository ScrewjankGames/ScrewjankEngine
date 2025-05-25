module;
#include <cmath>

export module sj.shared.math:Vec2;

export namespace sj
{
    class Vec2
    {
    public:
        constexpr Vec2() = default;

        constexpr Vec2(float x, float y) : m_elements {x, y}
        {
        }

        constexpr auto&& operator[](this auto&& self, int idx) // -> float& or const float&
        {
            return self.m_elements[idx];
        }

        [[nodiscard]] constexpr float GetX() const
        {
            return m_elements[0];
        }

        [[nodiscard]] constexpr float GetY() const
        {
            return m_elements[1];
        }

        [[nodiscard]] constexpr Vec2 operator*(float s) const
        {
            return {m_elements[0] * s, m_elements[1] * s};
        }

        [[nodiscard]] constexpr Vec2 operator/(float s) const
        {
            return (*this) * (1/s);
        }

        constexpr inline bool operator==(const Vec2& other) const
        {
            return m_elements[0] == other.m_elements[0] && m_elements[1] == other.m_elements[1];
        }

    private:
        float m_elements[2];
    };

    inline constexpr Vec2 Vec2_Zero = Vec2(0.0f, 0.0f);
    inline constexpr Vec2 Vec2_UnitX = Vec2(1.0f, 0.0f);
    inline constexpr Vec2 Vec2_UnitY = Vec2(0.0f, 0.1f);

    [[nodiscard]] constexpr float MagnitudeSqr(const Vec2& v)
    {
        return (v.GetX() * v.GetX()) + (v.GetY() * v.GetY());
    }

    [[nodiscard]] constexpr float Magnitude(const Vec2& v)
    {
        return std::sqrtf(MagnitudeSqr(v));
    }

    [[nodiscard]] constexpr Vec2 Normalized(const Vec2& v)
    {
        return v / Magnitude(v);
    }
} // namespace sj
