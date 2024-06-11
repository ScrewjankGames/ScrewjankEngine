#pragma once

#include <ScrewjankShared/Math/Vec4.hpp>
#include <ScrewjankShared/Math/Vec3.hpp>
#include <ScrewjankShared/Math/IdentityTag.hpp>

namespace sj
{
    class alignas(16) Mat44 
    {
    public:
        Mat44() = default;
        Mat44(IdentityTagT);
        Mat44(Vec4 x, Vec4 y, Vec4 z, Vec4 w);

        auto&& operator[](this auto&& self, int idx) // -> Vec4& or const Vec4&
        {
            return self.m_rows[idx];
        }

        const Vec4& GetX() const;
        const Vec4& GetY() const;
        const Vec4& GetZ() const;
        const Vec4& GetW() const;

        Vec4 GetCol(int idx) const;

    private:
        Vec4 m_rows[4];
    };

    Vec4 operator*(const Vec4& vec, const Mat44& m);

    Mat44 operator*(const Mat44& a, const Mat44& b);

    Mat44 operator*(const Mat44& m, float s);
    inline Mat44 operator*(float s, const Mat44& m) { return m * s; };

    Mat44 operator+(const Mat44& a, const Mat44& b);

    [[nodiscard]] Mat44 AffineInverse(const Mat44& m);
   
    [[nodiscard]] Mat44 FromEulerXYZ(const Vec3& eulers);
}
