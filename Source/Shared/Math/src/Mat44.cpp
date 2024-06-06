#include <ScrewjankShared/Math/Mat44.hpp>

namespace sj
{
    Mat44::Mat44(IdentityTagT) 
        : m_rows
          {
            Vec4(1, 0, 0, 0),
            Vec4(0, 1, 0, 0),
            Vec4(0, 0, 1, 0),
            Vec4(0, 0, 0, 1)
          }
    {

    }

    Mat44::Mat44(Vec4 x, Vec4 y, Vec4 z, Vec4 w) 
		: m_rows {x, y, z, w}
	{
    }

    const Vec4& Mat44::GetX() const
    {
        return m_rows[0];
    }

    const Vec4& Mat44::GetY() const
    {
        return m_rows[1];
    }

    const Vec4& Mat44::GetZ() const
    {
        return m_rows[2];
    }

    const Vec4& Mat44::GetW() const
    {
        return m_rows[3];
    }

    Vec4 Mat44::GetCol(int idx) const
    {
        return Vec4(m_rows[0][idx], m_rows[1][idx], m_rows[2][idx], m_rows[3][idx]);
    }

    Vec4 operator*(const Vec4& v, const Mat44& m)
    {
        float xPrime = (v[0] * m[0][0]) + (v[1] * m[1][0]) + (v[2] * m[2][0]) + (v[3] * m[3][0]);
        float yPrime = (v[0] * m[0][1]) + (v[1] * m[1][1]) + (v[2] * m[2][1]) + (v[3] * m[3][1]);
        float zPrime = (v[0] * m[0][2]) + (v[1] * m[1][2]) + (v[2] * m[2][2]) + (v[3] * m[3][2]);
        float wPrime = (v[0] * m[0][3]) + (v[1] * m[1][3]) + (v[2] * m[2][3]) + (v[3] * m[3][3]);

        return Vec4(xPrime, yPrime, zPrime, wPrime);
    }

    Mat44 operator*(const Mat44& a, const Mat44& b)
    {
        const Vec4 aX = a.GetX();
        const Vec4 aY = a.GetY();
        const Vec4 aZ = a.GetZ();
        const Vec4 aW = a.GetW();

        return Mat44 
        {
            {aX.Dot(b.GetCol(0)), aX.Dot(b.GetCol(1)), aX.Dot(b.GetCol(2)), aX.Dot(b.GetCol(3))}, 
            {aY.Dot(b.GetCol(0)), aY.Dot(b.GetCol(1)), aY.Dot(b.GetCol(2)), aY.Dot(b.GetCol(3))}, 
            {aZ.Dot(b.GetCol(0)), aZ.Dot(b.GetCol(1)), aZ.Dot(b.GetCol(2)), aZ.Dot(b.GetCol(3))}, 
            {aW.Dot(b.GetCol(0)), aW.Dot(b.GetCol(1)), aW.Dot(b.GetCol(2)), aW.Dot(b.GetCol(3))}
        };
    }

    Mat44 AffineInverse(const Mat44& m)
    {
        const float invScaleX = 1.0f / Magnitude(m.GetX());
        const float invScaleY = 1.0f / Magnitude(m.GetY());
        const float invScaleZ = 1.0f / Magnitude(m.GetZ());

        const Vec4 unitX = m.GetX() * invScaleX;
        const Vec4 unitY = m.GetY() * invScaleY;
        const Vec4 unitZ = m.GetZ() * invScaleZ;

        Mat44 inverseRot {{unitX[0] * invScaleX, unitY[0] * invScaleX, unitZ[0] * invScaleX, 0},
                          {unitX[1] * invScaleY, unitY[1] * invScaleY, unitZ[1] * invScaleY, 0},
                          {unitX[2] * invScaleZ, unitY[2] * invScaleZ, unitZ[2] * invScaleZ, 0},
                          {0.0f,     0.0f,     0.0f,     1}};

        Vec4 inverseT = ( -m.GetW() ) * inverseRot;

        return Mat44(
            inverseRot.GetX(), 
            inverseRot.GetY(), 
            inverseRot.GetZ(), 
            inverseT);
    }
}