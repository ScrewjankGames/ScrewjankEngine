// Parent Include
#include <ScrewjankShared/Math/Vec2.hpp>

// STD Includes
#include <cmath>

namespace sj
{
    Vec2 Vec2::Zero = {0.0f, 0.0f};
    Vec2 Vec2::UnitX = {1.0f, 0.0f};
    Vec2 Vec2::UnitY = {0.0f, 1.0f};

	Vec2::Vec2(float x, float y) : m_elements {x, y}
	{

	}

	float Vec2::GetX() const
    {
        return (*this)[0];
    }

	float Vec2::GetY() const
    {
        return (*this)[1];
    }

    Vec2 Vec2::operator*(float s) const
    {
        return Vec2(GetX() * s, GetY() * s);
    }

    Vec2 Vec2::operator/(float s) const
    {
        return Vec2(GetX() / s, GetY() / s);
    }

    float Magnitude(const Vec2& v)
    {
        return std::sqrtf(MagnitudeSqr(v));
    }

    float MagnitudeSqr(const Vec2& v)
    {
        return (v.GetX() * v.GetX()) + (v.GetY() * v.GetY());
    }

    Vec2 Normalized(const Vec2& v)
    {
        return v / Magnitude(v);
    }
} // namespace sj