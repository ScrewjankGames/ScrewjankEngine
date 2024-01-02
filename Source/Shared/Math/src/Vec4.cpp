// Parent Include
#include <ScrewjankShared/Math/Vec4.hpp>

// Screwjank Includes
#include <ScrewjankShared/Math/Vec2.hpp>

// STD Includes
#include <cmath>

namespace sj
{
    const Vec4 Vec4::UnitX(1, 0, 0, 0);
    const Vec4 Vec4::UnitY(0, 1, 0, 0);
    const Vec4 Vec4::UnitZ(0, 0, 1, 0);
    const Vec4 Vec4::UnitW(0, 0, 0, 1);

    const Vec4 Vec4::Right = UnitX;
    const Vec4 Vec4::Up = UnitY;
    const Vec4 Vec4::Forward = -UnitZ;

    Vec4::Vec4(float x, float y, float z, float w) : m_elements{x, y, z, w}
    {
    }

    Vec4::Vec4(const Vec2& v, float z, float w) 
        : m_elements {v[0], v[1], z, w}
    {

    }

    Vec4 Vec4::operator*(const float s) const
    {
        return Vec4(
            m_elements[0] * s, 
            m_elements[1] * s, 
            m_elements[2] * s, 
            m_elements[3] * s
        );
    }

    Vec4 Vec4::operator/(const float s) const
    {
        return Vec4(
            m_elements[0] / s,
            m_elements[1] / s, 
            m_elements[2] / s,
            m_elements[3] / s
        );
    }

    Vec4 Vec4::operator-(const Vec4& other) const
    {
        return Vec4(
            m_elements[0] - other[0],
            m_elements[1] - other[1], 
            m_elements[2] - other[2],
            m_elements[3] - other[3]
        );
    }
    
    Vec4 Vec4::operator-() const
    {
        return (*this) * -1;
    }

    bool Vec4::operator==(const Vec4& other) const
    {
        return 
            (m_elements[0] == other.m_elements[0]) && 
            (m_elements[1] == other.m_elements[1]) &&
            (m_elements[2] == other.m_elements[2]) && 
            (m_elements[3] == other.m_elements[3]);
    }

    float Vec4::Dot(const Vec4& other) const
    {
        return 
            (m_elements[0] * other.m_elements[0]) + 
            (m_elements[1] * other.m_elements[1]) +
            (m_elements[2] * other.m_elements[2]) + 
            (m_elements[3] * other.m_elements[3]);
    }

    float Vec4::Magnitude() const
    {
        return std::sqrtf(this->Dot(*this));
    }

    Vec4 Normalize(const Vec4& v)
    {
        return v / v.Magnitude();
    }

    Vec4 Normalize3_W0(const Vec4& v)
    {
        Vec4 ret = Normalize(v);
        ret[3] = 0;
        return ret;
    }

    Vec4 Vec4::Cross( const Vec4& b) const
    {
        return Vec4(
            (m_elements[1] * b.m_elements[2]) - (m_elements[2] * b.m_elements[1]), 
            (m_elements[2] * b.m_elements[0]) - (m_elements[0] * b.m_elements[2]), 
            (m_elements[0] * b.m_elements[1]) - (m_elements[1] * b.m_elements[0]), 
            0
        );
    }

    float Vec4::GetX() const
    {
        return m_elements[0];
    }

    float Vec4::GetY() const
    {
        return m_elements[1];
    }

    float Vec4::GetZ() const
    {
        return m_elements[2];
    }

    float Vec4::GetW() const
    {
        return m_elements[3];
    }

} // namespace sj