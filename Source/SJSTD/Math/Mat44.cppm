module;

#include <algorithm>
#include <cmath>
#include <numbers>
#include <array>

export module sj.std.math:mat44;
import :sqt;
import :Vec3;
import :Vec4;
import :Tags;

export namespace sj
{
class mat44;

constexpr mat44 operator*(float s, const mat44& m);
constexpr vec4 operator*(const vec4& v, const mat44& m);
constexpr mat44 operator*(const mat44& a, const mat44& b);
constexpr mat44 operator*(const mat44& m, float s);
constexpr mat44 operator+(const mat44& a, const mat44& b);

class alignas(16) mat44
{
public:
    constexpr mat44() = default;
    constexpr mat44(IdentityTagT)
        : m_rows {vec4(1, 0, 0, 0), vec4(0, 1, 0, 0), vec4(0, 0, 1, 0), vec4(0, 0, 0, 1)}
    {
    }

    constexpr mat44(vec4 x, vec4 y, vec4 z, vec4 w) : m_rows {x, y, z, w}
    {
    }

    template <int tRow>
    [[nodiscard]] constexpr auto get_row() const -> vec4
    {
        static_assert(tRow >= 0 && tRow <= 3, "Row index out of range!");
        return m_rows[tRow];
    }

    template <int tCol>
    [[nodiscard]] constexpr auto get_col() const -> vec4
    {
        static_assert(tCol >= 0 && tCol <= 3, "Column index OOR");

        return {m_rows[0].get<tCol>(),
                m_rows[1].get<tCol>(),
                m_rows[2].get<tCol>(),
                m_rows[3].get<tCol>()};
    }

    template <int tRow>
    mat44& set_row(vec4 v)
    {
        static_assert(tRow >= 0 && tRow <= 3, "Row index out of range!");
        m_rows[tRow] = v;
        return *this;
    }

    template <int tRow, int tCol>
    [[nodiscard]] constexpr auto get() const -> float
    {
        return get_row<tRow>().template get<tCol>();
    }

    template <int tRow, int tCol>
    constexpr auto set(float value) -> mat44&
    {
        static_assert(tRow >= 0 && tRow <= 3, "Row index out of range!");
        m_rows[tRow].set<tCol>(value);
        return *this;
    }

    [[nodiscard]] constexpr const vec4& get_x() const
    {
        return m_rows[0];
    }

    [[nodiscard]] constexpr const vec4& get_y() const
    {
        return m_rows[1];
    }

    [[nodiscard]] constexpr const vec4& get_z() const
    {
        return m_rows[2];
    }

    [[nodiscard]] constexpr const vec4& get_w() const
    {
        return m_rows[3];
    }

    [[nodiscard]] auto get_unsigned_scale() const -> vec4
    {
        return vec4(get_x().magnitude(), get_y().magnitude(), get_z().magnitude(), 0.0f);
    }

    template <int tRow, int tCol>
    [[nodiscard]] constexpr auto set(float val) const -> void
    {
        static_assert(tRow > 0 && tRow <= 3, "Row index out of bounds");
        static_assert(tCol > 0 && tCol <= 3, "Column index out of bounds");

        m_rows[tRow].set<tCol>(val);
    }

    constexpr auto set_x(vec4 v) -> mat44&
    {
        m_rows[0] = v;
        return *this;
    }

    constexpr auto set_y(vec4 v) -> mat44&
    {
        m_rows[1] = v;
        return *this;
    }

    constexpr auto set_z(vec4 v) -> mat44&
    {
        m_rows[2] = v;
        return *this;
    }

    constexpr auto set_w(vec4 v) -> mat44&
    {
        m_rows[3] = v;
        return *this;
    }

    constexpr auto set_rot_euler_xyz(const vec3& eulers) -> mat44&
    {
        float xScale = m_rows[0].magnitude();
        float yScale = m_rows[1].magnitude();
        float zScale = m_rows[2].magnitude();

        mat44 rotation = mat44::from_euler_xyz(eulers);
        m_rows[0] = rotation.m_rows[0] * xScale;
        m_rows[1] = rotation.m_rows[1] * yScale;
        m_rows[2] = rotation.m_rows[2] * zScale;

        return *this;
    }

    constexpr auto set_rot_euler_zyx(const vec3& eulers) -> mat44&
    {
        float xScale = m_rows[0].magnitude();
        float yScale = m_rows[1].magnitude();
        float zScale = m_rows[2].magnitude();

        mat44 rotation = mat44::from_euler_zyx(eulers);
        m_rows[0] = rotation.m_rows[0] * xScale;
        m_rows[1] = rotation.m_rows[1] * yScale;
        m_rows[2] = rotation.m_rows[2] * zScale;

        return *this;
    }

    constexpr auto set_rot_euler_yxz(const vec3& eulers) -> mat44&
    {
        float xScale = m_rows[0].magnitude();
        float yScale = m_rows[1].magnitude();
        float zScale = m_rows[2].magnitude();

        mat44 rotation = mat44::from_euler_yxz(eulers);
        m_rows[0] = rotation.m_rows[0] * xScale;
        m_rows[1] = rotation.m_rows[1] * yScale;
        m_rows[2] = rotation.m_rows[2] * zScale;

        return *this;
    }

    [[nodiscard]] auto affine_inverse() const -> mat44
    {
        const float invScaleX = 1.0f / get_x().magnitude();
        const float invScaleY = 1.0f / get_y().magnitude();
        const float invScaleZ = 1.0f / get_z().magnitude();

        const vec4 unitX = get_x() * invScaleX;
        const vec4 unitY = get_y() * invScaleY;
        const vec4 unitZ = get_z() * invScaleZ;

        mat44 inverseRotScale {
            {unitX.get_x() * invScaleX, unitY.get_x() * invScaleY, unitZ.get_x() * invScaleZ, 0},
            {unitX.get_y() * invScaleX, unitY.get_y() * invScaleY, unitZ.get_y() * invScaleZ, 0},
            {unitX.get_z() * invScaleX, unitY.get_z() * invScaleY, unitZ.get_z() * invScaleZ, 0},
            {0.0f, 0.0f, 0.0f, 1}};

        vec4 inverseT = (-get_w()) * inverseRotScale;
        inverseT.set_w(1.0f);

        return {inverseRotScale.get_x(),
                inverseRotScale.get_y(),
                inverseRotScale.get_z(),
                inverseT};
    }

    static auto from_rotate_x(float x) -> mat44
    {
        return mat44 {
            {1.0f, 0.0f, 0.0f, 0.0f},
            {0.0f, std::cosf(x), std::sinf(x), 0.0f},
            {0.0f, -std::sinf(x), std::cosf(x), 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
        };
    }

    static auto from_rotate_y(float y) -> mat44
    {
        return mat44 {
            {std::cosf(y), 0.0f, -std::sinf(y), 0.0f},
            {0.0f, 1.0f, 0.0f, 0.0f},
            {std::sinf(y), 0.0f, std::cosf(y), 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
        };
    }

    static auto from_rotate_z(float z) -> mat44
    {
        return mat44 {
            {std::cosf(z), std::sinf(z), 0.0f, 0.0f},
            {-std::sinf(z), std::cosf(z), 0.0f, 0.0f},
            {0.0f, 0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
        };
    }

    // W is interpreted as angle spun about axis
    [[nodiscard]] static auto from_axis_angle(const vec4& axisAndAngle) -> mat44
    {
        const float sinTheta = std::sinf(axisAndAngle.get_w());
        const float cosTheta = std::cosf(axisAndAngle.get_w());
        const float versine = 1.0f - cosTheta;

        const float kx = axisAndAngle.get_x();
        const float ky = axisAndAngle.get_y();
        const float kz = axisAndAngle.get_z();

        return mat44(vec4(cosTheta + kx * kx * versine,
                          kx * ky * versine + kz * sinTheta,
                          kx * kz * versine - ky * sinTheta,
                          0.0f),
                     vec4(ky * kx * versine - kz * sinTheta,
                          cosTheta + ky * ky * versine,
                          ky * kz * versine + kx * sinTheta,
                          0.0f),
                     vec4(kz * kx * versine + ky * sinTheta,
                          kz * ky * versine - kx * sinTheta,
                          cosTheta + kz * kz * versine,
                          0.0f),
                     vec4(0.0f, 0.0f, 0.0f, 1.0f));
    }

    [[nodiscard]] static auto from_euler_xyz(const vec3& eulers) -> mat44
    {
        mat44 x = from_rotate_x(eulers.x);
        mat44 y = from_rotate_y(eulers.y);
        mat44 z = from_rotate_z(eulers.z);

        return x * y * z;
    }

    [[nodiscard]] static auto from_euler_xyz(const vec3& eulers, const vec4& translation) -> mat44
    {
        mat44 output = from_euler_xyz(eulers);
        output.set_w(translation);
        return output;
    }

    [[nodiscard]] static auto from_euler_zyx(const vec3& eulers) -> mat44
    {
        mat44 x = from_rotate_x(eulers.x);
        mat44 y = from_rotate_y(eulers.y);
        mat44 z = from_rotate_z(eulers.z);

        return z * y * x;
    }

    [[nodiscard]] static auto from_euler_zyx(const vec3& eulers, const vec4& translation) -> mat44
    {
        mat44 output = from_euler_zyx(eulers);
        output.set_w(translation);
        return output;
    }

    [[nodiscard]] static auto from_euler_yxz(const vec3& eulers) -> mat44
    {
        mat44 pitch = from_rotate_x(eulers.x);
        mat44 yaw = from_rotate_y(eulers.y);
        mat44 roll = from_rotate_z(eulers.z);

        return yaw * pitch * roll;
    }

    [[nodiscard]] static auto from_euler_yxz(const vec3& eulers, const vec4& translation) -> mat44
    {
        mat44 output = from_euler_yxz(eulers);
        output.set_w(translation);
        return output;
    }

    [[nodiscard]] static auto from_quat(const quat& q) -> mat44
    {
        const float x = q.get_x();
        const float y = q.get_y();
        const float z = q.get_z();
        const float w = q.get_w();

        vec4 row0(1.0f - 2.0f * y * y - 2.0f * z * z,
                  2.0f * x * y + 2.0f * z * w,
                  2.0f * x * z - 2.0f * y * w,
                  0.0f);
        vec4 row1(2.0f * x * y - 2.0f * z * w,
                  1.0f - 2.0f * x * x - 2.0f * z * z,
                  2.0f * y * z + 2.0f * x * w,
                  0.0f);
        vec4 row2(2.0f * x * z + 2.0f * y * w,
                  2.0f * y * z - 2.0f * x * w,
                  1.0f - 2.0f * x * x - 2.0f * y * y,
                  0.0f);
        vec4 row3(0.0f, 0.0f, 0.0f, 1.0f);
        return mat44(row0, row1, row2, row3);
    }

    [[nodiscard]] static auto from_sqt(const sqt& sqt) -> mat44
    {
        mat44 t = from_quat(sqt.q);
        t.set_row<0>(t.get_row<0>() * sqt.s.get_x());
        t.set_row<1>(t.get_row<1>() * sqt.s.get_y());
        t.set_row<2>(t.get_row<2>() * sqt.s.get_z());
        t.set_row<3>(sqt.t);
        t.set<3, 3>(1.0f);
        return t;
    }

    [[nodiscard]] constexpr auto get_euler_angles_xyz() const -> vec3
    {
        vec4 xAxis = m_rows[0].normalize3_w0();
        vec4 yAxis = m_rows[1].normalize3_w0();
        vec4 zAxis = m_rows[2].normalize3_w0();

        float sinY = std::clamp(-xAxis.get<2>(), -1.0f, 1.0f);

        if(sinY < 0.9999f && sinY > -0.9999f)
        {
            float yRot = std::asin(sinY);

            float xRot = std::atan2(yAxis.get<2>(), zAxis.get<2>());
            float zRot = std::atan2(xAxis.get<1>(), xAxis.get<0>());

            return sj::vec3 {.x = xRot, .y = yRot, .z = zRot};
        }
        else
        {
            constexpr float pi_over_2 = std::numbers::pi_v<float> / 2.0f;
            float zRot = 0;
            if(sinY >= 0.9999f)
            {
                float yRot = pi_over_2;
                float xRot = std::atan2(yAxis.get<0>(), zAxis.get<0>());
                return sj::vec3 {.x = xRot, .y = yRot, .z = zRot};
            }
            else
            {
                float yRot = -pi_over_2;
                float xRot = std::atan2(-yAxis.get<0>(), -zAxis.get<0>());
                return sj::vec3 {.x = xRot, .y = yRot, .z = zRot};
            }
        }
    }

    [[nodiscard]] auto get_euler_angles_yxz() const -> vec3
    {
        constexpr float pi_over_2 = std::numbers::pi_v<float> / 2.0f;

        vec4 xAxis = m_rows[0].normalize3_w0();
        vec4 yAxis = m_rows[1].normalize3_w0();
        vec4 zAxis = m_rows[2].normalize3_w0();
        mat44 noScale(xAxis, yAxis, zAxis, vec4(0, 0, 0, 1));

        float sinX = noScale.get<1, 2>();

        if(sinX < 0.9999f && sinX > -0.9999f)
        {
            return vec3(std::asin(sinX),
                        std::atan2(-noScale.get<0, 2>(), noScale.get<2, 2>()),
                        std::atan2(-noScale.get<1, 0>(), noScale.get<1, 1>()));
        }
        else
        {
            if(sinX >= 0.9999f)
            {
                return vec3(pi_over_2, std::atan2(noScale.get<0, 1>(), noScale.get<0, 0>()), 0.0f);
            }
            else
            {
                return vec3(-pi_over_2, std::atan2(-noScale.get<0, 1>(), noScale.get<0, 0>()), 0.0f);
            }
        }

    }

    [[nodiscard]] auto&& data(this auto&& self) //-> std::array<(const?)vec4, 4>&
    {
        return self.m_rows;
    }

    [[nodiscard]] mat44 normalized() const
    {
        return mat44 {get_x().normalize(), get_y().normalize(), get_z().normalize(), get_w()};
    }

private:
    std::array<vec4, 4> m_rows;
};

constexpr mat44 operator*(float s, const mat44& m)
{
    return m * s;
};

constexpr vec4 operator*(const vec4& v, const mat44& m)
{
    float xPrime = (v.get_x() * m.get<0, 0>())
                   + (v.get_y() * m.get<1, 0>())
                   + (v.get_z() * m.get<2, 0>())
                   + (v.get_w() * m.get<3, 0>());
    float yPrime = (v.get_x() * m.get<0, 1>())
                   + (v.get_y() * m.get<1, 1>())
                   + (v.get_z() * m.get<2, 1>())
                   + (v.get_w() * m.get<3, 1>());
    float zPrime = (v.get_x() * m.get<0, 2>())
                   + (v.get_y() * m.get<1, 2>())
                   + (v.get_z() * m.get<2, 2>())
                   + (v.get_w() * m.get<3, 2>());
    float wPrime = (v.get_x() * m.get<0, 3>())
                   + (v.get_y() * m.get<1, 3>())
                   + (v.get_z() * m.get<2, 3>())
                   + (v.get_w() * m.get<3, 3>());

    return {xPrime, yPrime, zPrime, wPrime};
}

constexpr mat44 operator*(const mat44& a, const mat44& b)
{
    const vec4 aX = a.get_x();
    const vec4 aY = a.get_y();
    const vec4 aZ = a.get_z();
    const vec4 aW = a.get_w();

    return mat44 {{aX.dot(b.get_col<0>()),
                   aX.dot(b.get_col<1>()),
                   aX.dot(b.get_col<2>()),
                   aX.dot(b.get_col<3>())},
                  {aY.dot(b.get_col<0>()),
                   aY.dot(b.get_col<1>()),
                   aY.dot(b.get_col<2>()),
                   aY.dot(b.get_col<3>())},
                  {aZ.dot(b.get_col<0>()),
                   aZ.dot(b.get_col<1>()),
                   aZ.dot(b.get_col<2>()),
                   aZ.dot(b.get_col<3>())},
                  {aW.dot(b.get_col<0>()),
                   aW.dot(b.get_col<1>()),
                   aW.dot(b.get_col<2>()),
                   aW.dot(b.get_col<3>())}};
}

constexpr mat44 operator*(const mat44& m, float s)
{
    return {m.get_row<0>() * s, m.get_row<1>() * s, m.get_row<2>() * s, m.get_row<3>() * s};
}

constexpr mat44 operator+(const mat44& a, const mat44& b)
{
    return {a.get_row<0>() + b.get_row<0>(),
            a.get_row<1>() + b.get_row<1>(),
            a.get_row<2>() + b.get_row<2>(),
            a.get_row<3>() + b.get_row<3>()};
}

[[nodiscard]] mat44 constexpr build_transform(const vec4 scale,
                                              const vec3& eulers,
                                              const vec4& translation)
{
    mat44 r = mat44::from_euler_xyz(eulers);

    mat44 s = mat44 {
        {scale.get<0>(), 0, 0, 0},
        {0, scale.get<1>(), 0, 0},
        {0, 0, scale.get<2>(), 0},
        {0, 0, 0, 1},
    };

    mat44 t(Vec4_UnitX, Vec4_UnitY, Vec4_UnitZ, translation);

    return s * r * t;
}

} // namespace sj
