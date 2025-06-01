module;

#include <cmath>
#include <array>

export module sj.shared.math:Vec4;
import :Vec2;
import :Vec3;

export namespace sj
{
    // Forward Declares
    class Vec2;
    class Vec3;
    class Quat;

    class alignas(16) Vec4
    {
    public:
        constexpr Vec4() = default;
        constexpr Vec4(const Vec2& v, float z = 0.0f, float w = 0.0f)
            : m_elements {v.GetX(), v.GetY(), z, w}
        {
        }

        constexpr Vec4(const Vec3& v, float w = 0.0f) : m_elements {v[0], v[1], v[2], w}
        {
        }

        constexpr Vec4(float x, float y, float z, float w) : m_elements {x, y, z, w}
        {
        }

        template<int tCol>
        [[nodiscard]] constexpr auto Get() const -> float
        {
            static_assert(tCol >= 0 && tCol <= 3, "Index out of range");

            return m_elements[tCol];
        }

        template<int tCol>
        constexpr auto Set(float v) -> Vec4&
        {
            static_assert(tCol >= 0 && tCol <= 3, "Index out of range");

            m_elements[tCol] = v;
            return *this;
        }

        constexpr Vec4& operator+=(const Vec4& other)
        {
            m_elements[0] += other.m_elements[0];
            m_elements[1] += other.m_elements[1];
            m_elements[2] += other.m_elements[2];
            m_elements[3] += other.m_elements[3];

            return *this;
        }

        [[nodiscard]] constexpr Vec4 operator/(const float s) const
        {
            return {m_elements[0] / s, m_elements[1] / s, m_elements[2] / s, m_elements[3] / s};
        }

        [[nodiscard]] constexpr Vec4 operator-(const Vec4& other) const
        {
            return {m_elements[0] - other.m_elements[0],
                    m_elements[1] - other.m_elements[1],
                    m_elements[2] - other.m_elements[2],
                    m_elements[3] - other.m_elements[3]};
        }

        constexpr Vec4& operator*=(float scalar)
        {
            m_elements[0] *= scalar;
            m_elements[1] *= scalar;
            m_elements[2] *= scalar;
            m_elements[3] *= scalar;

            return *this;
        }

        [[nodiscard]] constexpr Vec4 operator-() const
        {
            return {GetX() * -1.0f, GetY() * -1.0f, GetZ() * -1.0f, GetW() * -1.0f};
        }

        [[nodiscard]] constexpr bool operator==(const Vec4& other) const
        {
            return (m_elements[0] == other.m_elements[0]) &&
                   (m_elements[1] == other.m_elements[1]) &&
                   (m_elements[2] == other.m_elements[2]) && (m_elements[3] == other.m_elements[3]);
        }

        [[nodiscard]] constexpr float Dot(const Vec4& other) const
        {
            return (m_elements[0] * other.m_elements[0]) + (m_elements[1] * other.m_elements[1]) +
                   (m_elements[2] * other.m_elements[2]) + (m_elements[3] * other.m_elements[3]);
        }

        [[nodiscard]] constexpr Vec4 Cross(const Vec4& b) const
        {
            return {(m_elements[1] * b.m_elements[2]) - (m_elements[2] * b.m_elements[1]),
                    (m_elements[2] * b.m_elements[0]) - (m_elements[0] * b.m_elements[2]),
                    (m_elements[0] * b.m_elements[1]) - (m_elements[1] * b.m_elements[0]),
                    0};
        }

        [[nodiscard]] constexpr float GetX() const
        {
            return m_elements[0];
        }

        [[nodiscard]] constexpr float GetY() const
        {
            return m_elements[1];
        }

        [[nodiscard]] constexpr float GetZ() const
        {
            return m_elements[2];
        }

        [[nodiscard]] constexpr float GetW() const
        {
            return m_elements[3];
        }

        constexpr Vec4& SetX(float x) 
        {
            m_elements[0] = x;
            return *this;
        }

        constexpr Vec4& SetY(float y)
        {
            m_elements[1] = y;
            return *this;
        }

        constexpr Vec4& SetZ(float z)
        {
            m_elements[2] = z;
            return *this;
        }

        constexpr Vec4 SetW(float w)
        {
            m_elements[3] = w;
            return *this;
        }

        [[nodiscard]] auto Data() -> std::array<float, 4>&
        {
            return m_elements;
        }

        [[nodiscard]] auto Data() const -> const std::array<float, 4>&
        {
            return m_elements;
        }

    private:
        std::array<float, 4> m_elements = {};
    };

    constexpr inline Vec4 Vec4_UnitX = Vec4(1, 0, 0, 0);
    constexpr inline Vec4 Vec4_UnitY = Vec4(0, 1, 0, 0);
    constexpr inline Vec4 Vec4_UnitZ = Vec4(0, 0, 1, 0);
    constexpr inline Vec4 Vec4_UnitW = Vec4(0, 0, 0, 1);
    constexpr inline Vec4 Vec4_Right = Vec4_UnitX;
    constexpr inline Vec4 Vec4_Up = Vec4_UnitY;
    constexpr inline Vec4 Vec4_Forward = -Vec4_UnitZ;

    [[nodiscard]] constexpr Vec4 operator*(const Vec4& v, const float s)
    {
        return {v.GetX() * s, v.GetY() * s, v.GetZ() * s, v.GetW() * s};
    }

    [[nodiscard]] constexpr Vec4 operator*(const float s, const Vec4& v)
    {
        return v * s;
    }

    [[nodiscard]] constexpr Vec4 operator+(const Vec4& a, const Vec4& b)
    {
        return {a.GetX() + b.GetX(), a.GetY() + b.GetY(), a.GetZ() + b.GetZ(), a.GetW() + b.GetW()};
    }

    [[nodiscard]] constexpr float MagnitudeSqr(const Vec4& v)
    {
        return v.Dot(v);
    }

    [[nodiscard]] float Magnitude(const Vec4& v)
    {
        return std::sqrtf(MagnitudeSqr(v));
    }

    [[nodiscard]] Vec4 Normalize(const Vec4& v)
    {
        return v / Magnitude(v);
    }

    [[nodiscard]] Vec4 Normalize3_W0(const Vec4& v)
    {
        Vec4 ret = Normalize(v);
        ret.SetW(0);
        return ret;
    }

} // namespace sj
