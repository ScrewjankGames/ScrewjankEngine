#pragma once

#include <ScrewjankShared/Math/IdentityTag.hpp>

namespace sj
{
    // Forward Declares
    class Vec4;

    class alignas(16) Quat
    {
    public:
        Quat() = default;
        Quat(IdentityTagT);
        Quat(float x, float y, float z, float w);
        Quat(const Vec4& v);

        auto&& operator[](this auto&& self, int idx) // -> float& or const float&
        {
            return self.m_elements[idx];
        }

    private:
        float m_elements[4] = {};
    };
}