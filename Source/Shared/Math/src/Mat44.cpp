// Parent Include
#include <ScrewjankShared/Math/Mat44.hpp>

// STD Includes
#include <cmath>
#include <numbers>

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


    Vec3 Mat44::GetEulerAngles() const
    {
        Vec4 xAxis = Normalize3_W0(m_rows[0]);
        Vec4 yAxis = Normalize3_W0(m_rows[1]);
        Vec4 zAxis = Normalize3_W0(m_rows[2]);

        if(!(zAxis[0] == 1 || zAxis[0] == -1))
        {
            float yRot = std::asin(zAxis[0]);
            float invCosY = 1.0f/ std::cos(yRot);

            float xRot = std::atan2(zAxis[1] * invCosY , zAxis[2] * invCosY);
            float zRot = std::atan2(yAxis[0], xAxis[0]);

            return sj::Vec3(xRot, yRot, zRot);
        }
        else
        {
            float zRot = 0;
            if(zAxis[0] == -1)
            {
                float yRot = std::numbers::pi_v<float> / 2.0f;
                float xRot = std::atan2(xAxis[1], xAxis[2]);
                return sj::Vec3(xRot, yRot, zRot);
            }
            else
            {
                float yRot = -std::numbers::pi_v<float> / 2.0f;
                float xRot = std::atan2(-xAxis[1], -xAxis[2]);
                return sj::Vec3(xRot, yRot, zRot);
            }
        }
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

    Mat44 operator*(const Mat44& m, float s)
    {
        return Mat44(
            m[0] * s, 
            m[1] * s, 
            m[2] * s, 
            m[3] * s
        );
    }

    Mat44 operator+(const Mat44& a, const Mat44& b)
    {
        return Mat44(
            a[0] + b[0], 
            a[1] + b[1], 
            a[2] + b[2],
            a[3] + b[3]
        );
    }

    Mat44 Mat44::AffineInverse(const Mat44& m)
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
        inverseT[3] = 1.0f;

        return Mat44(
            inverseRot.GetX(), 
            inverseRot.GetY(), 
            inverseRot.GetZ(), 
            inverseT);
    }

    Mat44 Mat44::FromEulerXYZ(const Vec3& eulers)
    {
        Mat44 x {
            {1.0f, 0.0f, 0.0f, 0.0f},
            {0.0f, std::cosf(eulers[0]), std::sinf(eulers[0]), 0.0f},
            {0.0f, -std::sinf(eulers[0]), std::cosf(eulers[0]), 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
        };

        Mat44 y {
            {std::cosf(eulers[1]), 0.0f, -std::sinf(eulers[1]), 0.0f},
            {0.0f, 1.0f, 0.0f, 0.0f},
            {std::sinf(eulers[1]), 0.0f, std::cosf(eulers[1]), 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
        };

        Mat44 z {
            {std::cosf(eulers[2]), std::sinf(eulers[2]), 0.0f, 0.0f},
            {-std::sinf(eulers[2]), std::cosf(eulers[2]), 0.0f, 0.0f},
            {0.0f, 0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
        };

        return x * y * z;
    }

    Mat44 Mat44::FromEulerXYZ(const Vec3& eulers, const Vec4& translation)
    {
        Mat44 output = FromEulerXYZ(eulers);
        output[3] = translation;
        return output;
    }

    Mat44 BuildTransform(const Vec4 scale, const Vec3& eulers, const Vec4& translation)
    {
        Mat44 r = Mat44::FromEulerXYZ(eulers);
        
        Mat44 s = Mat44 {
            {scale[0], 0, 0, 0},
            {0, scale[1], 0, 0},
            {0, 0, scale[2], 0},
            {0, 0, 0, 1},
        };

        Mat44 t(Vec4::UnitX, Vec4::UnitY, Vec4::UnitZ, translation);

        return s * r * t;
    }
}