module;

#include <cmath>
#include <numbers>
#include <array>

export module sj.std.math:Mat44;
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

        template <int tRow>
        [[nodiscard]] constexpr auto GetRow() const -> Vec4
        {
            static_assert(tRow >= 0 && tRow <= 3, "Row index out of range!");
            return m_rows[tRow];
        }

        template <int tCol>
        [[nodiscard]] constexpr auto GetCol() const -> Vec4
        {
            static_assert(tCol >= 0 && tCol <= 3, "Column index OOR");

            return {m_rows[0].Get<tCol>(),
                    m_rows[1].Get<tCol>(),
                    m_rows[2].Get<tCol>(),
                    m_rows[3].Get<tCol>()};
        }

        template<int tRow>
        Mat44& SetRow(Vec4 v)
        {
            static_assert(tRow >= 0 && tRow <= 3, "Row index out of range!");
            m_rows[tRow] = v;
            return *this;
        }

        template <int tRow, int tCol>
        [[nodiscard]] constexpr auto Get() const -> float
        {
            return GetRow<tRow>().template Get<tCol>();
        }

        template <int tRow, int tCol>
        constexpr auto Set(float value) -> Mat44&
        {
            m_rows[tRow].Set<tCol>(value);
            return *this;
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

        constexpr auto SetX(Vec4 v) -> Mat44&
        {
            m_rows[0] = v;
            return *this;
        }

        constexpr auto SetY(Vec4 v) -> Mat44&
        {
            m_rows[1] = v;
            return *this;
        }

        constexpr auto SetZ(Vec4 v) -> Mat44&
        {
            m_rows[2] = v;
            return *this;
        }

        constexpr auto SetW(Vec4 v) -> Mat44&
        {
            m_rows[3] = v;
            return *this;
        }

        [[nodiscard]] auto AffineInverse() const -> Mat44
        {
            const float invScaleX = 1.0f / GetX().Magnitude();
            const float invScaleY = 1.0f / GetY().Magnitude();
            const float invScaleZ = 1.0f / GetZ().Magnitude();

            const Vec4 unitX = GetX() * invScaleX;
            const Vec4 unitY = GetY() * invScaleY;
            const Vec4 unitZ = GetZ() * invScaleZ;

            Mat44 inverseRot {
                {unitX.GetX() * invScaleX, unitY.GetX() * invScaleX, unitZ.GetX() * invScaleX, 0},
                {unitX.GetY() * invScaleY, unitY.GetY() * invScaleY, unitZ.GetY() * invScaleY, 0},
                {unitX.GetZ() * invScaleZ, unitY.GetZ() * invScaleZ, unitZ.GetZ() * invScaleZ, 0},
                {0.0f, 0.0f, 0.0f, 1}};

            Vec4 inverseT = (-GetW()) * inverseRot;
            inverseT.SetW(1.0f);

            return {inverseRot.GetX(), inverseRot.GetY(), inverseRot.GetZ(), inverseT};
        }

        [[nodiscard]] static auto FromEulerXYZ(const Vec3& eulers) -> Mat44
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

        [[nodiscard]] static auto FromEulerXYZ(const Vec3& eulers, const Vec4& translation) -> Mat44
        {
            Mat44 output = FromEulerXYZ(eulers);
            output.SetW(translation);
            return output;
        }

        [[nodiscard]] constexpr auto GetEulerAngles() const -> Vec3
        {
            Vec4 xAxis = m_rows[0].Normalize3_W0();
            Vec4 yAxis = m_rows[1].Normalize3_W0();
            Vec4 zAxis = m_rows[2].Normalize3_W0();

            if(!(xAxis.Get<2>() == 1 || xAxis.Get<2>() == -1))
            {
                float yRot = -std::asin(xAxis.Get<2>());
                float invCosY = 1.0f / std::cos(yRot);

                float xRot = std::atan2(yAxis.Get<2>() * invCosY, zAxis.Get<2>() * invCosY);
                float zRot = std::atan2(xAxis.Get<1>() * invCosY, xAxis.Get<0>() * invCosY);

                return sj::Vec3 {.x = xRot, .y = yRot, .z = zRot};
            }
            else
            {
                constexpr float pi_over_2 = std::numbers::pi_v<float> / 2.0f;
                float zRot = 0;
                if(xAxis.Get<2>() == -1)
                {
                    float yRot = pi_over_2;
                    float xRot = std::atan2(yAxis.Get<0>(), zAxis.Get<0>());
                    return sj::Vec3 {.x = xRot, .y = yRot, .z = zRot};
                }
                else
                {
                    float yRot = -pi_over_2;
                    float xRot = std::atan2(-yAxis.Get<0>(), -zAxis.Get<0>());
                    return sj::Vec3 {.x = xRot, .y = yRot, .z = zRot};
                }
            }
        }

        [[nodiscard]] auto Data() -> std::array<Vec4, 4>&
        {
            return m_rows;
        }

        [[nodiscard]] auto Data() const -> const std::array<Vec4, 4>&
        {
            return m_rows;
        }
        
    private:
        std::array<Vec4, 4> m_rows;
    };

    constexpr Mat44 operator*(float s, const Mat44& m)
    {
        return m * s;
    };

    constexpr Vec4 operator*(const Vec4& v, const Mat44& m)
    {
        float xPrime = (v.GetX() * m.Get<0, 0>()) + (v.GetY() * m.Get<1, 0>()) +
                       (v.GetZ() * m.Get<2, 0>()) + (v.GetW() * m.Get<3, 0>());
        float yPrime = (v.GetX() * m.Get<0, 1>()) + (v.GetY() * m.Get<1, 1>()) +
                       (v.GetZ() * m.Get<2, 1>()) + (v.GetW() * m.Get<3, 1>());
        float zPrime = (v.GetX() * m.Get<0, 2>()) + (v.GetY() * m.Get<1, 2>()) +
                       (v.GetZ() * m.Get<2, 2>()) + (v.GetW() * m.Get<3, 2>());
        float wPrime = (v.GetX() * m.Get<0, 3>()) + (v.GetY() * m.Get<1, 3>()) +
                       (v.GetZ() * m.Get<2, 3>()) + (v.GetW() * m.Get<3, 3>());

        return {xPrime, yPrime, zPrime, wPrime};
    }

    constexpr Mat44 operator*(const Mat44& a, const Mat44& b)
    {
        const Vec4 aX = a.GetX();
        const Vec4 aY = a.GetY();
        const Vec4 aZ = a.GetZ();
        const Vec4 aW = a.GetW();

        return Mat44 {{aX.Dot(b.GetCol<0>()),
                       aX.Dot(b.GetCol<1>()),
                       aX.Dot(b.GetCol<2>()),
                       aX.Dot(b.GetCol<3>())},
                      {aY.Dot(b.GetCol<0>()),
                       aY.Dot(b.GetCol<1>()),
                       aY.Dot(b.GetCol<2>()),
                       aY.Dot(b.GetCol<3>())},
                      {aZ.Dot(b.GetCol<0>()),
                       aZ.Dot(b.GetCol<1>()),
                       aZ.Dot(b.GetCol<2>()),
                       aZ.Dot(b.GetCol<3>())},
                      {aW.Dot(b.GetCol<0>()),
                       aW.Dot(b.GetCol<1>()),
                       aW.Dot(b.GetCol<2>()),
                       aW.Dot(b.GetCol<3>())}};
    }

    constexpr Mat44 operator*(const Mat44& m, float s)
    {
        return {m.GetRow<0>() * s, m.GetRow<1>() * s, m.GetRow<2>() * s, m.GetRow<3>() * s};
    }

    constexpr Mat44 operator+(const Mat44& a, const Mat44& b)
    {
        return {a.GetRow<0>() + b.GetRow<0>(),
                a.GetRow<1>() + b.GetRow<1>(),
                a.GetRow<2>() + b.GetRow<2>(),
                a.GetRow<3>() + b.GetRow<3>()};
    }

    [[nodiscard]] Mat44 constexpr BuildTransform(const Vec4 scale,
                                                 const Vec3& eulers,
                                                 const Vec4& translation)
    {
        Mat44 r = Mat44::FromEulerXYZ(eulers);

        Mat44 s = Mat44 {
            {scale.Get<0>(), 0, 0, 0},
            {0, scale.Get<1>(), 0, 0},
            {0, 0, scale.Get<2>(), 0},
            {0, 0, 0, 1},
        };

        Mat44 t(Vec4_UnitX, Vec4_UnitY, Vec4_UnitZ, translation);

        return s * r * t;
    }

} // namespace sj
