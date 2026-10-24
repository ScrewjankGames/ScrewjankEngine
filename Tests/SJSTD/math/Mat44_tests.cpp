// STD Headers
#include <numbers>
#include <cmath>

// Library Headers
#include "gtest/gtest.h"

import sj.std.math;

using namespace sj;

namespace math_tests
{

namespace
{
void ExpectVec4Near(const vec4& actual, const vec4& expected, float eps = 1e-4f)
{
    EXPECT_NEAR(actual.get_x(), expected.get_x(), eps);
    EXPECT_NEAR(actual.get_y(), expected.get_y(), eps);
    EXPECT_NEAR(actual.get_z(), expected.get_z(), eps);
    EXPECT_NEAR(actual.get_w(), expected.get_w(), eps);
}

void ExpectMat44Near(const mat44& actual, const mat44& expected, float eps = 1e-4f)
{
    for(int r = 0; r < 4; ++r)
    {
        for(int c = 0; c < 4; ++c)
        {
            EXPECT_NEAR(actual.data()[r].data()[c], expected.data()[r].data()[c], eps)
                << "Mismatch at row " << r << ", col " << c;
        }
    }
}

void ExpectVec3Near(const vec3& actual, const vec3& expected, float eps = 1e-3f)
{
    EXPECT_NEAR(actual.x, expected.x, eps);
    EXPECT_NEAR(actual.y, expected.y, eps);
    EXPECT_NEAR(actual.z, expected.z, eps);
}
} // namespace

// =============================================================================
// Construction & Element Access
// =============================================================================

TEST(Mat44Tests, DefaultConstructorZeroInitializes)
{
    mat44 m;
    for(int r = 0; r < 4; ++r)
    {
        for(int c = 0; c < 4; ++c)
        {
            EXPECT_FLOAT_EQ(m.data()[r].data()[c], 0.0f);
        }
    }
}

TEST(Mat44Tests, IdentityConstructor)
{
    mat44 m(kIdentityTag);

    ExpectVec4Near(m.get_row<0>(), vec4(1.0f, 0.0f, 0.0f, 0.0f));
    ExpectVec4Near(m.get_row<1>(), vec4(0.0f, 1.0f, 0.0f, 0.0f));
    ExpectVec4Near(m.get_row<2>(), vec4(0.0f, 0.0f, 1.0f, 0.0f));
    ExpectVec4Near(m.get_row<3>(), vec4(0.0f, 0.0f, 0.0f, 1.0f));

    ExpectVec4Near(m.get_x(), Vec4_UnitX);
    ExpectVec4Near(m.get_y(), Vec4_UnitY);
    ExpectVec4Near(m.get_z(), Vec4_UnitZ);
    ExpectVec4Near(m.get_w(), Vec4_UnitW);

    EXPECT_FLOAT_EQ((m.get<0, 0>()), 1.0f);
    EXPECT_FLOAT_EQ((m.get<1, 1>()), 1.0f);
    EXPECT_FLOAT_EQ((m.get<2, 2>()), 1.0f);
    EXPECT_FLOAT_EQ((m.get<3, 3>()), 1.0f);

    EXPECT_FLOAT_EQ((m.get<0, 1>()), 0.0f);
    EXPECT_FLOAT_EQ((m.get<1, 0>()), 0.0f);
}

TEST(Mat44Tests, ComponentConstructorAndAccessors)
{
    vec4 r0(1.0f, 2.0f, 3.0f, 4.0f);
    vec4 r1(5.0f, 6.0f, 7.0f, 8.0f);
    vec4 r2(9.0f, 10.0f, 11.0f, 12.0f);
    vec4 r3(13.0f, 14.0f, 15.0f, 16.0f);

    mat44 m(r0, r1, r2, r3);

    ExpectVec4Near(m.get_row<0>(), r0);
    ExpectVec4Near(m.get_row<1>(), r1);
    ExpectVec4Near(m.get_row<2>(), r2);
    ExpectVec4Near(m.get_row<3>(), r3);

    ExpectVec4Near(m.get_x(), r0);
    ExpectVec4Near(m.get_y(), r1);
    ExpectVec4Near(m.get_z(), r2);
    ExpectVec4Near(m.get_w(), r3);

    ExpectVec4Near(m.get_col<0>(), vec4(1.0f, 5.0f, 9.0f, 13.0f));
    ExpectVec4Near(m.get_col<1>(), vec4(2.0f, 6.0f, 10.0f, 14.0f));
    ExpectVec4Near(m.get_col<2>(), vec4(3.0f, 7.0f, 11.0f, 15.0f));
    ExpectVec4Near(m.get_col<3>(), vec4(4.0f, 8.0f, 12.0f, 16.0f));

    EXPECT_FLOAT_EQ((m.get<0, 0>()), 1.0f);
    EXPECT_FLOAT_EQ((m.get<0, 3>()), 4.0f);
    EXPECT_FLOAT_EQ((m.get<1, 2>()), 7.0f);
    EXPECT_FLOAT_EQ((m.get<2, 1>()), 10.0f);
    EXPECT_FLOAT_EQ((m.get<3, 0>()), 13.0f);
    EXPECT_FLOAT_EQ((m.get<3, 3>()), 16.0f);
}

TEST(Mat44Tests, Mutators)
{
    mat44 m(kIdentityTag);

    m.set_row<0>(vec4(2.0f, 3.0f, 4.0f, 5.0f));
    ExpectVec4Near(m.get_row<0>(), vec4(2.0f, 3.0f, 4.0f, 5.0f));

    m.set<1, 1>(7.5f);
    EXPECT_FLOAT_EQ((m.get<1, 1>()), 7.5f);

    m.set_x(vec4(10.0f, 0.0f, 0.0f, 0.0f));
    m.set_y(vec4(0.0f, 20.0f, 0.0f, 0.0f));
    m.set_z(vec4(0.0f, 0.0f, 30.0f, 0.0f));
    m.set_w(vec4(1.0f, 2.0f, 3.0f, 1.0f));

    ExpectVec4Near(m.get_x(), vec4(10.0f, 0.0f, 0.0f, 0.0f));
    ExpectVec4Near(m.get_y(), vec4(0.0f, 20.0f, 0.0f, 0.0f));
    ExpectVec4Near(m.get_z(), vec4(0.0f, 0.0f, 30.0f, 0.0f));
    ExpectVec4Near(m.get_w(), vec4(1.0f, 2.0f, 3.0f, 1.0f));
}

// =============================================================================
// Scaling and Normalization
// =============================================================================

TEST(Mat44Tests, GetUnsignedScale)
{
    mat44 m(vec4(3.0f, 0.0f, 0.0f, 0.0f),
            vec4(0.0f, -4.0f, 0.0f, 0.0f),
            vec4(0.0f, 0.0f, 5.0f, 0.0f),
            vec4(10.0f, 20.0f, 30.0f, 1.0f));

    vec4 scale = m.get_unsigned_scale();
    EXPECT_NEAR(scale.get_x(), 3.0f, 1e-4f);
    EXPECT_NEAR(scale.get_y(), 4.0f, 1e-4f);
    EXPECT_NEAR(scale.get_z(), 5.0f, 1e-4f);
    EXPECT_NEAR(scale.get_w(), 0.0f, 1e-4f);
}

TEST(Mat44Tests, Normalized)
{
    mat44 m(vec4(3.0f, 0.0f, 0.0f, 0.0f),
            vec4(0.0f, 4.0f, 0.0f, 0.0f),
            vec4(0.0f, 0.0f, 5.0f, 0.0f),
            vec4(1.0f, 2.0f, 3.0f, 1.0f));

    mat44 norm = m.normalized();
    ExpectVec4Near(norm.get_x(), Vec4_UnitX);
    ExpectVec4Near(norm.get_y(), Vec4_UnitY);
    ExpectVec4Near(norm.get_z(), Vec4_UnitZ);
    ExpectVec4Near(norm.get_w(), vec4(1.0f, 2.0f, 3.0f, 1.0f));
}

// =============================================================================
// Arithmetic Operators
// =============================================================================

TEST(Mat44Tests, ScalarMultiplicationAndAddition)
{
    mat44 identity(kIdentityTag);
    mat44 scaled1 = identity * 2.5f;
    mat44 scaled2 = 2.5f * identity;

    ExpectMat44Near(scaled1, scaled2);
    EXPECT_FLOAT_EQ((scaled1.get<0, 0>()), 2.5f);
    EXPECT_FLOAT_EQ((scaled1.get<1, 1>()), 2.5f);
    EXPECT_FLOAT_EQ((scaled1.get<2, 2>()), 2.5f);
    EXPECT_FLOAT_EQ((scaled1.get<3, 3>()), 2.5f);
    EXPECT_FLOAT_EQ((scaled1.get<0, 1>()), 0.0f);

    mat44 sum = identity + identity;
    ExpectMat44Near(sum, identity * 2.0f);
}

TEST(Mat44Tests, VectorMatrixMultiplication)
{
    mat44 identity(kIdentityTag);
    vec4 v(1.0f, 2.0f, 3.0f, 1.0f);
    vec4 result = v * identity;
    ExpectVec4Near(result, v);

    // Translation matrix: row vectors on left multiply matrix on right
    mat44 translation(Vec4_UnitX, Vec4_UnitY, Vec4_UnitZ, vec4(10.0f, -5.0f, 2.0f, 1.0f));
    vec4 point(0.0f, 0.0f, 0.0f, 1.0f);
    vec4 direction(0.0f, 0.0f, 1.0f, 0.0f);

    ExpectVec4Near(point * translation, vec4(10.0f, -5.0f, 2.0f, 1.0f));
    ExpectVec4Near(direction * translation, direction);
}

TEST(Mat44Tests, MatrixMultiplication)
{
    mat44 identity(kIdentityTag);
    mat44 a(vec4(1, 2, 3, 4), vec4(5, 6, 7, 8), vec4(9, 1, 2, 3), vec4(4, 5, 6, 7));

    // Identity law
    ExpectMat44Near(a * identity, a);
    ExpectMat44Near(identity * a, a);

    // Scale then translation composition
    mat44 scale(vec4(2, 0, 0, 0), vec4(0, 3, 0, 0), vec4(0, 0, 4, 0), vec4(0, 0, 0, 1));
    mat44 trans(Vec4_UnitX, Vec4_UnitY, Vec4_UnitZ, vec4(5, 6, 7, 1));

    mat44 combined = scale * trans;
    vec4 p(1, 1, 1, 1);
    ExpectVec4Near(p * combined, (p * scale) * trans);
    ExpectVec4Near(p * combined, vec4(7, 9, 11, 1));

    // Associativity: (A * B) * C == A * (B * C)
    mat44 b(vec4(2, 0, 1, 0), vec4(0, 1, 0, 2), vec4(1, 1, 0, 0), vec4(0, 0, 1, 1));
    mat44 c(vec4(1, 1, 0, 0), vec4(0, 1, 1, 0), vec4(0, 0, 1, 1), vec4(1, 0, 0, 1));
    ExpectMat44Near((a * b) * c, a * (b * c));
}

// =============================================================================
// Axis Rotations
// =============================================================================

TEST(Mat44Tests, FromRotateX)
{
    float angle = sj::to_rads(90.0f);
    mat44 rx = mat44::from_rotate_x(angle);

    vec4 x = Vec4_UnitX * rx;
    vec4 y = Vec4_UnitY * rx;
    vec4 z = Vec4_UnitZ * rx;

    ExpectVec4Near(x, Vec4_UnitX, 1e-4f);
    ExpectVec4Near(y, Vec4_UnitZ, 1e-4f);
    ExpectVec4Near(z, -Vec4_UnitY, 1e-4f);
}

TEST(Mat44Tests, FromRotateY)
{
    float angle = sj::to_rads(90.0f);
    mat44 ry = mat44::from_rotate_y(angle);

    vec4 x = Vec4_UnitX * ry;
    vec4 y = Vec4_UnitY * ry;
    vec4 z = Vec4_UnitZ * ry;

    ExpectVec4Near(x, -Vec4_UnitZ, 1e-4f);
    ExpectVec4Near(y, Vec4_UnitY, 1e-4f);
    ExpectVec4Near(z, Vec4_UnitX, 1e-4f);
}

TEST(Mat44Tests, FromRotateZ)
{
    float angle = sj::to_rads(90.0f);
    mat44 rz = mat44::from_rotate_z(angle);

    vec4 x = Vec4_UnitX * rz;
    vec4 y = Vec4_UnitY * rz;
    vec4 z = Vec4_UnitZ * rz;

    ExpectVec4Near(x, Vec4_UnitY, 1e-4f);
    ExpectVec4Near(y, -Vec4_UnitX, 1e-4f);
    ExpectVec4Near(z, Vec4_UnitZ, 1e-4f);
}

TEST(Mat44Tests, FromAxisAngle)
{
    float angle = sj::to_rads(90.0f);

    // Rotate about X axis
    mat44 ax = mat44::from_axis_angle(vec4(1.0f, 0.0f, 0.0f, angle));
    mat44 rx = mat44::from_rotate_x(angle);
    ExpectMat44Near(ax, rx);

    // Rotate about Y axis
    mat44 ay = mat44::from_axis_angle(vec4(0.0f, 1.0f, 0.0f, angle));
    mat44 ry = mat44::from_rotate_y(angle);
    ExpectMat44Near(ay, ry);

    // Rotate about Z axis
    mat44 az = mat44::from_axis_angle(vec4(0.0f, 0.0f, 1.0f, angle));
    mat44 rz = mat44::from_rotate_z(angle);
    ExpectMat44Near(az, rz);

    // Rotate 360 degrees around arbitrary unit axis -> Identity
    vec4 arbitraryAxis = vec4(1.0f, 2.0f, 3.0f, 0.0f).normalize();
    arbitraryAxis.set_w(sj::to_rads(360.0f));
    mat44 fullTurn = mat44::from_axis_angle(arbitraryAxis);
    ExpectMat44Near(fullTurn, mat44(kIdentityTag));
}

// =============================================================================
// Quaternion & SQT
// =============================================================================

TEST(Mat44Tests, FromQuat)
{
    // Identity quaternion
    quat idQuat(kIdentityTag);
    ExpectMat44Near(mat44::from_quat(idQuat), mat44(kIdentityTag));

    // 90 degrees around X: q = (sin(45 deg), 0, 0, cos(45 deg))
    float halfAngle = sj::to_rads(45.0f);
    quat qx(std::sin(halfAngle), 0.0f, 0.0f, std::cos(halfAngle));
    mat44 mqx = mat44::from_quat(qx);
    mat44 mrx = mat44::from_rotate_x(sj::to_rads(90.0f));
    ExpectMat44Near(mqx, mrx);

    // 90 degrees around Y
    quat qy(0.0f, std::sin(halfAngle), 0.0f, std::cos(halfAngle));
    mat44 mqy = mat44::from_quat(qy);
    mat44 mry = mat44::from_rotate_y(sj::to_rads(90.0f));
    ExpectMat44Near(mqy, mry);

    // 90 degrees around Z
    quat qz(0.0f, 0.0f, std::sin(halfAngle), std::cos(halfAngle));
    mat44 mqz = mat44::from_quat(qz);
    mat44 mrz = mat44::from_rotate_z(sj::to_rads(90.0f));
    ExpectMat44Near(mqz, mrz);
}

TEST(Mat44Tests, FromSQT)
{
    float halfAngle = sj::to_rads(45.0f); // 90 deg around Y
    sqt transform {
        .s = vec4(2.0f, 3.0f, 4.0f, 0.0f),
        .q = quat(0.0f, std::sin(halfAngle), 0.0f, std::cos(halfAngle)),
        .t = vec4(10.0f, 20.0f, 30.0f, 1.0f)
    };

    mat44 m = mat44::from_sqt(transform);

    vec4 scale = m.get_unsigned_scale();
    EXPECT_NEAR(scale.get_x(), 2.0f, 1e-4f);
    EXPECT_NEAR(scale.get_y(), 3.0f, 1e-4f);
    EXPECT_NEAR(scale.get_z(), 4.0f, 1e-4f);
    ExpectVec4Near(m.get_w(), vec4(10.0f, 20.0f, 30.0f, 1.0f));

    // Transform a point: scale (1, 1, 1) -> (2, 3, 4)
    // Rotate 90 deg around Y -> x'=z=4, z'=-x=-2 -> (4, 3, -2)
    // Translate -> (14, 23, 28, 1)
    vec4 p(1.0f, 1.0f, 1.0f, 1.0f);
    ExpectVec4Near(p * m, vec4(14.0f, 23.0f, 28.0f, 1.0f), 1e-4f);
}

// =============================================================================
// Euler Rotations & Extraction
// =============================================================================

TEST(Mat44Tests, FromEulerXYZRoundTrip)
{
    auto testInput = vec3(sj::to_rads(45.0f), sj::to_rads(30.0f), sj::to_rads(60.0f));
    mat44 m = mat44::from_euler_xyz(testInput);

    auto testOutput = m.get_euler_angles_xyz();

    ASSERT_NEAR(testInput.x, testOutput.x, 0.001f);
    ASSERT_NEAR(testInput.y, testOutput.y, 0.001f);
    ASSERT_NEAR(testInput.z, testOutput.z, 0.001f);
}

TEST(Mat44Tests, FromEulerXYZWithTranslation)
{
    vec3 eulers(sj::to_rads(20.0f), sj::to_rads(30.0f), sj::to_rads(40.0f));
    vec4 trans(5.0f, -2.0f, 8.0f, 1.0f);

    mat44 m = mat44::from_euler_xyz(eulers, trans);
    ExpectVec4Near(m.get_w(), trans);

    mat44 rotOnly = mat44::from_euler_xyz(eulers);
    ExpectVec4Near(m.get_x(), rotOnly.get_x());
    ExpectVec4Near(m.get_y(), rotOnly.get_y());
    ExpectVec4Near(m.get_z(), rotOnly.get_z());
}

TEST(Mat44Tests, FromEulerZYX)
{
    vec3 eulers(sj::to_rads(15.0f), sj::to_rads(25.0f), sj::to_rads(35.0f));
    mat44 rx = mat44::from_rotate_x(eulers.x);
    mat44 ry = mat44::from_rotate_y(eulers.y);
    mat44 rz = mat44::from_rotate_z(eulers.z);

    mat44 expected = rz * ry * rx;
    mat44 actual = mat44::from_euler_zyx(eulers);
    ExpectMat44Near(actual, expected);

    vec4 trans(1.0f, 2.0f, 3.0f, 1.0f);
    mat44 withTrans = mat44::from_euler_zyx(eulers, trans);
    ExpectVec4Near(withTrans.get_w(), trans);
}

TEST(Mat44Tests, FromEulerYXZRoundTrip)
{
    vec3 testInput(sj::to_rads(20.0f), sj::to_rads(35.0f), sj::to_rads(50.0f));
    mat44 m = mat44::from_euler_yxz(testInput);

    vec3 testOutput = m.get_euler_angles_yxz();
    ExpectVec3Near(testOutput, testInput, 0.001f);

    vec4 trans(3.0f, 4.0f, 5.0f, 1.0f);
    mat44 withTrans = mat44::from_euler_yxz(testInput, trans);
    ExpectVec4Near(withTrans.get_w(), trans);
}

TEST(Mat44Tests, FromEulerXYZGimbalLockPositive)
{
    // Y = +90 degrees -> Gimbal lock
    vec3 testInput(sj::to_rads(30.0f), sj::to_rads(90.0f), 0.0f);
    mat44 m = mat44::from_euler_xyz(testInput);

    vec3 testOutput = m.get_euler_angles_xyz();
    // In gimbal lock, the rotation represented by testOutput should produce the exact same matrix
    mat44 reconstructed = mat44::from_euler_xyz(testOutput);
    ExpectMat44Near(reconstructed, m, 0.01f);
}

TEST(Mat44Tests, FromEulerXYZGimbalLockNegative)
{
    // Y = -90 degrees -> Gimbal lock
    vec3 testInput(sj::to_rads(30.0f), -sj::to_rads(90.0f), 0.0f);
    mat44 m = mat44::from_euler_xyz(testInput);

    vec3 testOutput = m.get_euler_angles_xyz();
    mat44 reconstructed = mat44::from_euler_xyz(testOutput);
    ExpectMat44Near(reconstructed, m, 0.01f);
}

TEST(Mat44Tests, FromEulerYXZGimbalLockPositive)
{
    // X = +90 degrees -> Gimbal lock
    vec3 testInput(sj::to_rads(90.0f), sj::to_rads(40.0f), 0.0f);
    mat44 m = mat44::from_euler_yxz(testInput);

    vec3 testOutput = m.get_euler_angles_yxz();
    mat44 reconstructed = mat44::from_euler_yxz(testOutput);
    ExpectMat44Near(reconstructed, m, 0.01f);
}

TEST(Mat44Tests, FromEulerYXZGimbalLockNegative)
{
    // X = -90 degrees -> Gimbal lock
    vec3 testInput(-sj::to_rads(90.0f), sj::to_rads(40.0f), 0.0f);
    mat44 m = mat44::from_euler_yxz(testInput);

    vec3 testOutput = m.get_euler_angles_yxz();
    mat44 reconstructed = mat44::from_euler_yxz(testOutput);
    ExpectMat44Near(reconstructed, m, 0.01f);
}

TEST(Mat44Tests, FromEulerXYZNegativeAnglesRoundTrip)
{
    vec3 testInput(sj::to_rads(-25.0f), sj::to_rads(-40.0f), sj::to_rads(-55.0f));
    mat44 m = mat44::from_euler_xyz(testInput);

    vec3 testOutput = m.get_euler_angles_xyz();
    ExpectVec3Near(testOutput, testInput, 0.001f);
}

TEST(Mat44Tests, SetRotEulerPreservesScaleAndTranslation)
{
    vec4 trans(10.0f, 20.0f, 30.0f, 1.0f);
    mat44 m(vec4(2.0f, 0.0f, 0.0f, 0.0f),
            vec4(0.0f, 3.0f, 0.0f, 0.0f),
            vec4(0.0f, 0.0f, 4.0f, 0.0f),
            trans);

    vec3 newEulers(sj::to_rads(30.0f), sj::to_rads(45.0f), sj::to_rads(60.0f));

    // set_rot_euler_xyz
    m.set_rot_euler_xyz(newEulers);
    ExpectVec4Near(m.get_w(), trans);
    vec4 scaleXyz = m.get_unsigned_scale();
    EXPECT_NEAR(scaleXyz.get_x(), 2.0f, 1e-4f);
    EXPECT_NEAR(scaleXyz.get_y(), 3.0f, 1e-4f);
    EXPECT_NEAR(scaleXyz.get_z(), 4.0f, 1e-4f);
    ExpectVec3Near(m.get_euler_angles_xyz(), newEulers, 0.001f);

    // Verify matrix orientation matches unscaled from_euler_xyz
    mat44 rotXyz = mat44::from_euler_xyz(newEulers);
    ExpectVec4Near(m.get_x().normalize3_w0(), rotXyz.get_x());
    ExpectVec4Near(m.get_y().normalize3_w0(), rotXyz.get_y());
    ExpectVec4Near(m.get_z().normalize3_w0(), rotXyz.get_z());

    // set_rot_euler_yxz
    m.set_rot_euler_yxz(newEulers);
    ExpectVec4Near(m.get_w(), trans);
    vec4 scaleYxz = m.get_unsigned_scale();
    EXPECT_NEAR(scaleYxz.get_x(), 2.0f, 1e-4f);
    EXPECT_NEAR(scaleYxz.get_y(), 3.0f, 1e-4f);
    EXPECT_NEAR(scaleYxz.get_z(), 4.0f, 1e-4f);
    ExpectVec3Near(m.get_euler_angles_yxz(), newEulers, 0.001f);

    mat44 rotYxz = mat44::from_euler_yxz(newEulers);
    ExpectVec4Near(m.get_x().normalize3_w0(), rotYxz.get_x());
    ExpectVec4Near(m.get_y().normalize3_w0(), rotYxz.get_y());
    ExpectVec4Near(m.get_z().normalize3_w0(), rotYxz.get_z());

    // set_rot_euler_zyx
    m.set_rot_euler_zyx(newEulers);
    ExpectVec4Near(m.get_w(), trans);
    vec4 scaleZyx = m.get_unsigned_scale();
    EXPECT_NEAR(scaleZyx.get_x(), 2.0f, 1e-4f);
    EXPECT_NEAR(scaleZyx.get_y(), 3.0f, 1e-4f);
    EXPECT_NEAR(scaleZyx.get_z(), 4.0f, 1e-4f);

    mat44 rotZyx = mat44::from_euler_zyx(newEulers);
    ExpectVec4Near(m.get_x().normalize3_w0(), rotZyx.get_x());
    ExpectVec4Near(m.get_y().normalize3_w0(), rotZyx.get_y());
    ExpectVec4Near(m.get_z().normalize3_w0(), rotZyx.get_z());
}

// =============================================================================
// Build Transform & Affine Inverse
// =============================================================================

TEST(Mat44Tests, BuildTransform)
{
    vec4 scale(2.0f, 3.0f, 4.0f, 1.0f);
    vec3 eulers(sj::to_rads(30.0f), sj::to_rads(45.0f), sj::to_rads(60.0f));
    vec4 trans(7.0f, 8.0f, 9.0f, 1.0f);

    mat44 t = build_transform(scale, eulers, trans);

    // Verify scale
    vec4 actualScale = t.get_unsigned_scale();
    EXPECT_NEAR(actualScale.get_x(), 2.0f, 1e-4f);
    EXPECT_NEAR(actualScale.get_y(), 3.0f, 1e-4f);
    EXPECT_NEAR(actualScale.get_z(), 4.0f, 1e-4f);

    // Verify translation
    ExpectVec4Near(t.get_w(), trans);

    // Verify point transformation matches scale * rot * trans
    mat44 sMat(vec4(2, 0, 0, 0), vec4(0, 3, 0, 0), vec4(0, 0, 4, 0), vec4(0, 0, 0, 1));
    mat44 rMat = mat44::from_euler_xyz(eulers);
    mat44 tMat(Vec4_UnitX, Vec4_UnitY, Vec4_UnitZ, trans);
    mat44 expected = sMat * rMat * tMat;

    ExpectMat44Near(t, expected);
}

TEST(Mat44Tests, AffineInverseIdentity)
{
    mat44 id(kIdentityTag);
    ExpectMat44Near(id.affine_inverse(), id);
}

TEST(Mat44Tests, AffineInverseTranslationOnly)
{
    vec4 trans(5.0f, -3.0f, 12.0f, 1.0f);
    mat44 t(Vec4_UnitX, Vec4_UnitY, Vec4_UnitZ, trans);
    mat44 invT = t.affine_inverse();

    ExpectMat44Near(t * invT, mat44(kIdentityTag));
    ExpectMat44Near(invT * t, mat44(kIdentityTag));
    ExpectVec4Near(invT.get_w(), vec4(-5.0f, 3.0f, -12.0f, 1.0f));
}

TEST(Mat44Tests, AffineInverseRotationOnly)
{
    mat44 r = mat44::from_euler_xyz(vec3(sj::to_rads(30.0f), sj::to_rads(45.0f), sj::to_rads(60.0f)));
    mat44 invR = r.affine_inverse();

    ExpectMat44Near(r * invR, mat44(kIdentityTag));
    ExpectMat44Near(invR * r, mat44(kIdentityTag));
}

TEST(Mat44Tests, AffineInverseScaleRotationTranslation)
{
    vec4 scale(2.0f, 0.5f, 3.0f, 1.0f);
    vec3 eulers(sj::to_rads(25.0f), sj::to_rads(40.0f), sj::to_rads(55.0f));
    vec4 trans(10.0f, -15.0f, 20.0f, 1.0f);

    mat44 m = build_transform(scale, eulers, trans);
    mat44 invM = m.affine_inverse();

    ExpectMat44Near(m * invM, mat44(kIdentityTag), 1e-3f);
    ExpectMat44Near(invM * m, mat44(kIdentityTag), 1e-3f);

    // Transforming a point through M and then M_inv should return original point
    vec4 p(3.0f, -7.0f, 11.0f, 1.0f);
    vec4 transformed = (p * m) * invM;
    ExpectVec4Near(transformed, p, 1e-3f);
}

} // namespace math_tests
