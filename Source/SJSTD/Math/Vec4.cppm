module;

#include <cmath>
#include <array>

export module sj.std.math:Vec4;
import :Vec2;
import :Vec3;

export namespace sj
{
// Forward Declares
class vec2;
class vec3;
class quat;

class alignas(16) vec4
{
public:
    constexpr vec4() = default;
    constexpr vec4(const vec2& v, float z = 0.0f, float w = 0.0f)
        : m_elements {v.get_x(), v.get_y(), z, w}
    {
    }

    constexpr vec4(const vec3& v, float w = 0.0f) : m_elements {v[0], v[1], v[2], w}
    {
    }

    constexpr vec4(float x, float y, float z, float w) : m_elements {x, y, z, w}
    {
    }

    template <int tCol>
    [[nodiscard]] constexpr auto get() const -> float
    {
        static_assert(tCol >= 0 && tCol <= 3, "Index out of range");

        return m_elements[tCol];
    }

    template <int tCol>
    constexpr auto set(float v) -> vec4&
    {
        static_assert(tCol >= 0 && tCol <= 3, "Index out of range");

        m_elements[tCol] = v;
        return *this;
    }

    constexpr vec4& operator+=(const vec4& other)
    {
        m_elements[0] += other.m_elements[0];
        m_elements[1] += other.m_elements[1];
        m_elements[2] += other.m_elements[2];
        m_elements[3] += other.m_elements[3];

        return *this;
    }

    [[nodiscard]] constexpr vec4 operator/(const float s) const
    {
        return {m_elements[0] / s, m_elements[1] / s, m_elements[2] / s, m_elements[3] / s};
    }

    [[nodiscard]] constexpr vec4 operator-(const vec4& other) const
    {
        return {m_elements[0] - other.m_elements[0],
                m_elements[1] - other.m_elements[1],
                m_elements[2] - other.m_elements[2],
                m_elements[3] - other.m_elements[3]};
    }

    constexpr vec4& operator*=(float scalar)
    {
        m_elements[0] *= scalar;
        m_elements[1] *= scalar;
        m_elements[2] *= scalar;
        m_elements[3] *= scalar;

        return *this;
    }

    [[nodiscard]] constexpr vec4 operator-() const
    {
        return {get_x() * -1.0f, get_y() * -1.0f, get_z() * -1.0f, get_w() * -1.0f};
    }

    [[nodiscard]] constexpr bool operator==(const vec4& other) const
    {
        return (m_elements[0] == other.m_elements[0])
               && (m_elements[1] == other.m_elements[1])
               && (m_elements[2] == other.m_elements[2])
               && (m_elements[3] == other.m_elements[3]);
    }

    [[nodiscard]] constexpr float dot(const vec4& other) const
    {
        return (m_elements[0] * other.m_elements[0])
               + (m_elements[1] * other.m_elements[1])
               + (m_elements[2] * other.m_elements[2])
               + (m_elements[3] * other.m_elements[3]);
    }

    [[nodiscard]] constexpr vec4 cross(const vec4& b) const
    {
        return {(m_elements[1] * b.m_elements[2]) - (m_elements[2] * b.m_elements[1]),
                (m_elements[2] * b.m_elements[0]) - (m_elements[0] * b.m_elements[2]),
                (m_elements[0] * b.m_elements[1]) - (m_elements[1] * b.m_elements[0]),
                0};
    }

    [[nodiscard]] constexpr float get_x() const
    {
        return m_elements[0];
    }

    [[nodiscard]] constexpr float get_y() const
    {
        return m_elements[1];
    }

    [[nodiscard]] constexpr float get_z() const
    {
        return m_elements[2];
    }

    [[nodiscard]] constexpr float get_w() const
    {
        return m_elements[3];
    }

    constexpr vec4& set_x(float x)
    {
        m_elements[0] = x;
        return *this;
    }

    constexpr vec4& set_y(float y)
    {
        m_elements[1] = y;
        return *this;
    }

    constexpr vec4& set_z(float z)
    {
        m_elements[2] = z;
        return *this;
    }

    constexpr vec4& set_w(float w)
    {
        m_elements[3] = w;
        return *this;
    }

    [[nodiscard]] constexpr auto&& data(this auto&& self) // -> std::array<(const?) float, 4>&,
    {
        return std::forward<decltype(self)>(self).m_elements;
    }

    [[nodiscard]] constexpr auto magnitude_sqr() const -> float
    {
        return this->dot(*this);
    }

    [[nodiscard]] auto magnitude() const -> float
    {
        return std::sqrtf(this->magnitude_sqr());
    }

    [[nodiscard]] auto normalize() const -> vec4 // Not reference!
    {
        return *this / magnitude();
    }

    [[nodiscard]] auto normalize3() const -> vec4 // Not reference!
    {
        vec4 tmp = *this;
        tmp.set_w(0);
        return tmp / tmp.magnitude();
    }

    [[nodiscard]] auto normalize3_w0() const -> vec4 // Not reference!
    {
        vec4 tmp = *this;
        tmp.set_w(0);
        return tmp.normalize();
    }

private:
    std::array<float, 4> m_elements = {};
};

constexpr inline vec4 Vec4_UnitX = vec4(1, 0, 0, 0);
constexpr inline vec4 Vec4_UnitY = vec4(0, 1, 0, 0);
constexpr inline vec4 Vec4_UnitZ = vec4(0, 0, 1, 0);
constexpr inline vec4 Vec4_UnitW = vec4(0, 0, 0, 1);
constexpr inline vec4 Vec4_Right = Vec4_UnitX;
constexpr inline vec4 Vec4_Up = Vec4_UnitY;
constexpr inline vec4 Vec4_Forward = -Vec4_UnitZ;

[[nodiscard]] constexpr vec4 operator*(const vec4& v, const float s)
{
    return {v.get_x() * s, v.get_y() * s, v.get_z() * s, v.get_w() * s};
}

[[nodiscard]] constexpr vec4 operator*(const float s, const vec4& v)
{
    return v * s;
}

[[nodiscard]] constexpr vec4 operator+(const vec4& a, const vec4& b)
{
    return {a.get_x() + b.get_x(),
            a.get_y() + b.get_y(),
            a.get_z() + b.get_z(),
            a.get_w() + b.get_w()};
}

[[nodiscard]] constexpr vec4 operator*(const vec4& a, const vec4& b)
{
    return {a.get_x() * b.get_x(),
            a.get_y() * b.get_y(),
            a.get_z() * b.get_z(),
            a.get_w() * b.get_w()};
}

} // namespace sj
