module;

#include <cmath>
#include <numbers>

export module sj.shared.math:Mat44;
import :Vec3;
import :Vec4;
import :Tags;

export namespace sj
{
    class Mat44;

    constexpr Mat44 operator*(float s, const Mat44& m);
    constexpr Vec4 operator*(const Vec4& v, const Mat44& m);
    constexpr Mat44 operator*(const Mat44& a, const Mat44& b);
    constexpr Mat44 operator*(const Mat44& m, float s);
    constexpr Mat44 operator+(const Mat44& a, const Mat44& b);

    class alignas(16) Mat44
    {
    public:
        constexpr Mat44() = default;
        constexpr Mat44(IdentityTagT)
            : m_rows {Vec4(1, 0, 0, 0), Vec4(0, 1, 0, 0), Vec4(0, 0, 1, 0), Vec4(0, 0, 0, 1)}
        {
        }

        constexpr Mat44(Vec4 x, Vec4 y, Vec4 z, Vec4 w) : m_rows {x, y, z, w}
        {
        }

        constexpr auto&& operator[](this auto&& self, int idx) // -> Vec4& or const Vec4&
        {
            return self.m_rows[idx];
        }

        [[nodiscard]] constexpr const Vec4& GetX() const
        {
            return m_rows[0];
        }

        [[nodiscard]] constexpr const Vec4& GetY() const
        {
            return m_rows[1];
        }

        [[nodiscard]] constexpr const Vec4& GetZ() const
        {
            return m_rows[2];
        }

        [[nodiscard]] constexpr const Vec4& GetW() const
        {
            return m_rows[3];
        }

        [[nodiscard]] static Mat44 AffineInverse(const Mat44& m)
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
                              {0.0f, 0.0f, 0.0f, 1}};

            Vec4 inverseT = (-m.GetW()) * inverseRot;
            inverseT[3] = 1.0f;

            return {inverseRot.GetX(), inverseRot.GetY(), inverseRot.GetZ(), inverseT};
        }

        [[nodiscard]] static Mat44 FromEulerXYZ(const Vec3& eulers)
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

        [[nodiscard]] static Mat44 FromEulerXYZ(const Vec3& eulers, const Vec4& translation)
        {
            Mat44 output = FromEulerXYZ(eulers);
            output[3] = translation;
            return output;
        }

        [[nodiscard]] constexpr Vec4 GetCol(int idx) const
        {
            return {m_rows[0][idx], m_rows[1][idx], m_rows[2][idx], m_rows[3][idx]};
        }

        [[nodiscard]] constexpr Vec3 GetEulerAngles() const
        {
            Vec4 xAxis = Normalize3_W0(m_rows[0]);
            Vec4 yAxis = Normalize3_W0(m_rows[1]);
            Vec4 zAxis = Normalize3_W0(m_rows[2]);

            if(!(xAxis[2] == 1 || xAxis[2] == -1))
            {
                float yRot = -std::asin(xAxis[2]);
                float invCosY = 1.0f / std::cos(yRot);

                float xRot = std::atan2(yAxis[2] * invCosY, zAxis[2] * invCosY);
                float zRot = std::atan2(xAxis[1] * invCosY, xAxis[0] * invCosY);

                return sj::Vec3 {.x=xRot, .y=yRot, .z=zRot};
            }
            else
            {
                constexpr float pi_over_2 = std::numbers::pi_v<float> / 2.0f;
                float zRot = 0;
                if(xAxis[2] == -1)
                {
                    float yRot = pi_over_2;
                    float xRot = std::atan2(yAxis[0], zAxis[0]);
                    return sj::Vec3 {.x=xRot, .y=yRot, .z=zRot};
                }
                else
                {
                    float yRot = -pi_over_2;
                    float xRot = std::atan2(-yAxis[0], -zAxis[0]);
                    return sj::Vec3 {.x=xRot, .y=yRot, .z=zRot};
                }
            }
        }

    private:
        Vec4 m_rows[4];
    };

    constexpr Mat44 operator*(float s, const Mat44& m)
    {
        return m * s;
    };

    constexpr Vec4 operator*(const Vec4& v, const Mat44& m)
    {
        float xPrime = (v[0] * m[0][0]) + (v[1] * m[1][0]) + (v[2] * m[2][0]) + (v[3] * m[3][0]);
        float yPrime = (v[0] * m[0][1]) + (v[1] * m[1][1]) + (v[2] * m[2][1]) + (v[3] * m[3][1]);
        float zPrime = (v[0] * m[0][2]) + (v[1] * m[1][2]) + (v[2] * m[2][2]) + (v[3] * m[3][2]);
        float wPrime = (v[0] * m[0][3]) + (v[1] * m[1][3]) + (v[2] * m[2][3]) + (v[3] * m[3][3]);

        return {xPrime, yPrime, zPrime, wPrime};
    }

    constexpr Mat44 operator*(const Mat44& a, const Mat44& b)
    {
        const Vec4 aX = a.GetX();
        const Vec4 aY = a.GetY();
        const Vec4 aZ = a.GetZ();
        const Vec4 aW = a.GetW();

        return Mat44 {
            {aX.Dot(b.GetCol(0)), aX.Dot(b.GetCol(1)), aX.Dot(b.GetCol(2)), aX.Dot(b.GetCol(3))},
            {aY.Dot(b.GetCol(0)), aY.Dot(b.GetCol(1)), aY.Dot(b.GetCol(2)), aY.Dot(b.GetCol(3))},
            {aZ.Dot(b.GetCol(0)), aZ.Dot(b.GetCol(1)), aZ.Dot(b.GetCol(2)), aZ.Dot(b.GetCol(3))},
            {aW.Dot(b.GetCol(0)), aW.Dot(b.GetCol(1)), aW.Dot(b.GetCol(2)), aW.Dot(b.GetCol(3))}};
    }

    constexpr Mat44 operator*(const Mat44& m, float s)
    {
        return {m[0] * s, m[1] * s, m[2] * s, m[3] * s};
    }

    constexpr Mat44 operator+(const Mat44& a, const Mat44& b)
    {
        return {a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]};
    }

    [[nodiscard]] Mat44
    constexpr BuildTransform(const Vec4 scale, const Vec3& eulers, const Vec4& translation)
    {
        Mat44 r = Mat44::FromEulerXYZ(eulers);

        Mat44 s = Mat44 {
            {scale[0], 0, 0, 0},
            {0, scale[1], 0, 0},
            {0, 0, scale[2], 0},
            {0, 0, 0, 1},
        };

        Mat44 t(Vec4_UnitX, Vec4_UnitY, Vec4_UnitZ, translation);

        return s * r * t;
    }

} // namespace sj
