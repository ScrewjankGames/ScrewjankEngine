module;
#include <numbers>
#include <cmath>

export module sj.std.math:Helpers;
import :Vec4;
import :Mat44;
import :Quat;

export namespace sj
{
    namespace numbers
    {
        template <typename T>
        constexpr T pi_over_2 = std::numbers::pi_v<T> / 2;
    }

    template <class FloatType>
    inline constexpr FloatType ToRadians(FloatType degrees)
    {
        return degrees * std::numbers::pi_v<FloatType> / FloatType(180.0);
    }

    template <class FloatType>
    inline constexpr FloatType ToDegrees(FloatType radians)
    {
        return radians * 180.0f * std::numbers::inv_pi_v<FloatType>;
    }

    /**
     * @return Create a view matrix that looks at the given target.
     *
     * In terms of cameras, calculates the inverse of the camera matrix
     */
    [[nodiscard]] Mat44 LookAt(const Vec4& eye, const Vec4& target, const Vec4& up)
    {
        // Get components of camera rotation matrix
        const Vec4 z = Normalize(eye - target);
        const Vec4 x = Normalize(up.Cross(z));
        const Vec4 y = z.Cross(x);

        // Calculate inverse of camera matrix for camera to world matrix
        const Vec4 t = Vec4(-eye.Dot(x), -eye.Dot(y), -eye.Dot(z), 1.0f);

        // Transpose the rotation part
        return Mat44({x.GetX(), y.GetX(), z.GetX(), 0},
                     {x.GetY(), y.GetY(), z.GetY(), 0},
                     {x.GetZ(), y.GetZ(), z.GetZ(), 0},
                     {t.GetX(), t.GetY(), t.GetZ(), 1});
    }

    /**
     * Computes perpsective projection matrix
     * @param verticalFOV: Vertical FOV of the view frustrum
     * @param aspectRatio: Render surface width / height
     * @param near: Near render plane
     * @param far: Far render plane
     *
     * When you're smarter, see:
     * https://www.youtube.com/watch?v=U0_ONQQ5ZNM
     * https://www.youtube.com/watch?v=YO46x8fALzE
     */
    Mat44 PerspectiveProjection(float verticalFOV, float aspectRatio, float near, float far)
    {
        const float invTanHalfvFov = 1.0f / std::tan(verticalFOV / 2.0f);

        Mat44 res;
        res.Set<0, 0>(invTanHalfvFov / aspectRatio);
        res.Set<1, 1>(-invTanHalfvFov);
        res.Set<2, 2>(far / (near - far));
        res.Set<2, 3>(-1.0f);
        res.Set<3, 2>(-(near * far) / (far - near));

        return res;
    }
    /**
     * Computes the exponential map from R^3 (vector space) to S^3 (Unit Quaternion Space)
     * @param v This argument is interpereted as an axis of rotation scaled
     *        by the angle of rotation
     *
     *  Useful for turning angular velocities in to rotations
     *
     * These helped me understand it better:
     * https://thenumb.at/Exponential-Rotations/
     * http://pajarito.materials.cmu.edu/documents/Exp_map_rotations.pdf
     */
    inline const /*expr*/ float small_theta_epsilon =
        std::pow(std::numeric_limits<float>::epsilon(), 0.5f);

    Quat Exp_Q(const Vec4& v)
    {

        float theta = Magnitude(v);
        float halfTheta = theta / 2.0f;
        if(theta <= small_theta_epsilon)
        {
            Vec4 asVec = (0.5f + ((theta * theta) / 48)) * v;
            asVec.Set<3>(std::cosf(halfTheta));
            return Quat(asVec);
        }
        else
        {
            Vec4 asVec = (std::sinf(halfTheta) / theta) * v;
            asVec.Set<3>(std::cosf(halfTheta));
            return Quat(asVec);
        }
    }

    Mat44 Exp_M(const Vec4& v)
    {
        float theta = Magnitude(v);

        if(theta < small_theta_epsilon)
        {
            return Mat44(kIdentityTag);
        }
        else
        {
            const Vec4 omega = v / theta;

            Mat44 tensor {{0.0f, omega.Get<2>(), -omega.Get<1>(), 0.0f},
                          {-omega.Get<2>(), 0.0f, omega.Get<0>(), 0.0f},
                          {omega.Get<1>(), -omega.Get<0>(), 0.0f, 0.0f},
                          {0.0f, 0.0f, 0.0f, 0.0f}};

            Mat44 tensorSqr = tensor * tensor;

            Mat44 t1 = Mat44(kIdentityTag);
            Mat44 t2 = (std::sinf(theta) * tensor);
            Mat44 t3 = (1 - std::cosf(theta)) * tensorSqr;
            Mat44 res = t1 + t2 + t3;
            return res;
        }
    }
} // namespace sj

inline constexpr long double operator""_deg_2_rad(const long double degrees)
{
    return sj::ToRadians(degrees);
}