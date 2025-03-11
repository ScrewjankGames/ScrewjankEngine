#include <ScrewjankShared/Math/Vec3.hpp>

namespace sj
{
    Vec3& sj::Vec3::operator+=(const Vec3& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;

        return *this;
    }
}
