#pragma once

#include <ScrewjankShared/Math/Vec4.hpp>

namespace sj
{
    struct IdentityTag{};

    class alignas(16) Mat44 
    {
    public:
        Mat44() = default;
        Mat44(IdentityTag);
        Mat44(Vec4 x, Vec4 y, Vec4 z, Vec4 w);

        Vec4& operator[](int row);

        Vec4& X();
        Vec4& Y();
        Vec4& Z();
        Vec4& W();

    private:
        Vec4 m_rows[4];
    };
}
