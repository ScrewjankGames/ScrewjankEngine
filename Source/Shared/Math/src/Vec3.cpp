#include <ScrewjankShared/Math/Vec3.hpp>

namespace sj
{
	Vec3::Vec3(float x, float y, float z) : m_elements {x, y, z}
	{

	}

    Vec3& sj::Vec3::operator+=(const Vec3& other)
    {
        m_elements[0] += other[0];
        m_elements[1] += other[1];
        m_elements[2] += other[2];

        return *this;
    }
}
