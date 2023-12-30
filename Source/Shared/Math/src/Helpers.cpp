#include <ScrewjankShared/Math/Helpers.hpp>

namespace sj
{
    Mat44 LookAt(const Vec4& eye, const Vec4& target, const Vec4& up)
    {
        const Vec4 z = Normalize(eye - target);
        const Vec4 x = up.Cross(z);
        const Vec4 y = x.Cross(z);
        const Vec4 t = Vec4(eye.Dot(x), eye.Dot(y), eye.Dot(z), 1.0f);
        return Mat44(x, y, z, eye);
    }
}