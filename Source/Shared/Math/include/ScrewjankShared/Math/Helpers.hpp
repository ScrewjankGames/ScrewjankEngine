#pragma once

#include <ScrewjankShared/Math/Vec4.hpp>
#include <ScrewjankShared/Math/Mat44.hpp>

namespace sj
{
    Mat44 LookAt(const Vec4& eye, const Vec4& target, const Vec4& up);
}