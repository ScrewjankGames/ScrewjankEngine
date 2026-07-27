module;
#include <numbers>
#include <cmath>
#include <limits>
export module sj.std.math:Helpers;
import :Vec4;
import :mat44;
import :Quat;

export namespace sj
{
    namespace numbers
    {
        template <typename T>
        constexpr T pi_over_2 = std::numbers::pi_v<T> / 2;
    }

    template <class FloatType>
    inline constexpr FloatType to_rads(FloatType degrees)
    {
        return degrees * std::numbers::pi_v<FloatType> / FloatType(180.0);
    }

    template <class FloatType>
    inline constexpr FloatType to_degs(FloatType radians)
    {
        return radians * 180.0f * std::numbers::inv_pi_v<FloatType>;
    }

    /**
     * @return Create a view matrix that looks at the given target.
     *
     * In terms of cameras, calculates the inverse of the camera matrix
     */
    [[nodiscard]] mat44 compute_look_at(const vec4& eye, const vec4& target, const vec4& up)
    {
        // Get components of camera rotation matrix
        const vec4 z = (eye - target).normalize();
        const vec4 x = up.cross(z).normalize();
        const vec4 y = z.cross(x);

        // Calculate inverse of camera matrix for camera to world matrix
        const vec4 t = vec4(-eye.dot(x), -eye.dot(y), -eye.dot(z), 1.0f);

        // Transpose the rotation part
        return mat44({x.get_x(), y.get_x(), z.get_x(), 0},
                     {x.get_y(), y.get_y(), z.get_y(), 0},
                     {x.get_z(), y.get_z(), z.get_z(), 0},
                     {t.get_x(), t.get_y(), t.get_z(), 1});
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

    quat exp(const vec4& v)
    {
        float theta = v.magnitude();
        float halfTheta = theta / 2.0f;
        if(theta <= small_theta_epsilon)
        {
            vec4 asVec = (0.5f + ((theta * theta) / 48)) * v;
            asVec.set<3>(std::cosf(halfTheta));
            return quat(asVec);
        }
        else
        {
            vec4 asVec = (std::sinf(halfTheta) / theta) * v;
            asVec.set<3>(std::cosf(halfTheta));
            return quat(asVec);
        }
    }
} // namespace sj

inline constexpr long double operator""_deg_2_rad(const long double degrees)
{
    return sj::to_rads(degrees);
}