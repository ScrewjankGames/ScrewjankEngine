module;

#include <array>

export module sj.std.math:Quat;
import :Vec4;
import :Tags;

export namespace sj
{
class alignas(16) quat
{
public:
    quat() = default;

    quat(IdentityTagT _) : m_elements {0.0f, 0.0f, 0.0f, 1.0f}
    {
    }

    quat(float x, float y, float z, float w) : m_elements {x, y, z, w}
    {
    }

    quat(const vec4& v) : m_elements(v.data())
    {
    }

    auto&& operator[](this auto&& self, int idx) // -> float& or const float&
    {
        return self.m_elements[idx];
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

    constexpr quat& set_x(float x)
    {
        m_elements[0] = x;
        return *this;
    }

    constexpr quat& set_y(float y)
    {
        m_elements[1] = y;
        return *this;
    }

    constexpr quat& set_z(float z)
    {
        m_elements[2] = z;
        return *this;
    }

    constexpr quat& set_w(float w)
    {
        m_elements[3] = w;
        return *this;
    }

private:
    std::array<float, 4> m_elements;
};
} // namespace sj