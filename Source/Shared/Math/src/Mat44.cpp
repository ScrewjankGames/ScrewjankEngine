#include <ScrewjankShared/Math/Mat44.hpp>

namespace sj
{
    Mat44::Mat44(IdentityTag) 
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

    Vec4& Mat44::X()
    {
        return m_rows[0];
    }

    Vec4& Mat44::Y()
    {
        return m_rows[1];
    }

    Vec4& Mat44::Z()
    {
        return m_rows[2];
    }

    Vec4& Mat44::W()
    {
        return m_rows[3];
    }

    Vec4 operator*(const Vec4& v, const Mat44& m)
    {
        float xPrime = (v[0] * m[0][0]) + (v[1] * m[1][0]) + (v[2] * m[2][0]) + (v[3] * m[3][0]);
        float yPrime = (v[0] * m[0][1]) + (v[1] * m[1][1]) + (v[2] * m[2][1]) + (v[3] * m[3][1]);
        float zPrime = (v[0] * m[0][2]) + (v[1] * m[1][2]) + (v[2] * m[2][2]) + (v[3] * m[3][2]);
        float wPrime = (v[0] * m[0][3]) + (v[1] * m[1][3]) + (v[2] * m[2][3]) + (v[3] * m[3][3]);

        return Vec4(xPrime, yPrime, zPrime, wPrime);
    }
}