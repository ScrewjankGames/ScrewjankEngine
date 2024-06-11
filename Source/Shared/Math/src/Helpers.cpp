// Parent Include
#include <ScrewjankShared/Math/Helpers.hpp>

// STD Includes
#include <cmath>
#include <limits>

namespace sj
{
    Mat44 LookAt(const Vec4& eye, const Vec4& target, const Vec4& up)
    {
        // Get components of camera rotation matrix
        const Vec4 z = Normalize(eye - target);
        const Vec4 x = Normalize(up.Cross(z));
        const Vec4 y = z.Cross(x);

        // Calculate inverse of camera matrix for camera to world matrix
        const Vec4 t = Vec4(-eye.Dot(x), -eye.Dot(y), -eye.Dot(z), 1.0f);

        // Transpose the rotation part
        return Mat44(
                { x.GetX(), y.GetX(), z.GetX(), 0 }, 
                { x.GetY(), y.GetY(), z.GetY(), 0 },
                { x.GetZ(), y.GetZ(), z.GetZ(), 0 },
                { t.GetX(), t.GetY(), t.GetZ(), 1 }
        );
    }

    /**
     * When you're smarter, see:
     * https://www.youtube.com/watch?v=U0_ONQQ5ZNM
     * https://www.youtube.com/watch?v=YO46x8fALzE
     */
    Mat44 PerspectiveProjection(float verticalFOV, float aspectRatio, float near, float far)
    {
        const float invTanHalfvFov = 1.0f / std::tan(verticalFOV / 2.0f);
        const float depth = far - near;
        const float invDepth = 1.0f / depth;

        Mat44 res;
        res[0][0] = invTanHalfvFov / aspectRatio;
        res[1][1] = -invTanHalfvFov;
        res[2][2] = far / (near - far);
        res[2][3] = -1.0f;
        res[3][2] = -(near * far) / (far - near);

        return res;
    }

    Mat44 ToAngularVelocityTensor(const Vec4& omega)
    {
        /**
         * https://en.wikipedia.org/wiki/Angular_velocity_tensor
         * https://www.youtube.com/watch?v=zJJldJYMxVU
         */
        return Mat44(
            {0, -omega[3], omega[2], 0}, 
            {omega[3], 0, -omega[1], 0}, 
            {-omega[2], -omega[1], 0, 0}, 
            {0,0,0,1}
        );
    }
    
    static const float small_theta_epsilon = std::pow(std::numeric_limits<float>::epsilon(), 0.5f);

    Quat Exp_Q(const Vec4& v)
    {

        float theta = Magnitude(v);
        float halfTheta = theta / 2.0f;
        if(theta <= small_theta_epsilon)
        {
            Vec4 asVec = (0.5f + ((theta * theta) / 48)) * v;
            asVec[3] = std::cosf(halfTheta);
            return Quat(asVec);
        }
        else
        {
            Vec4 asVec = (std::sinf(halfTheta) / theta) * v;
            asVec[3] = std::cosf(halfTheta);
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

            Mat44 tensor {
                {     0.0f,  omega[2], -omega[1], 0.0f}, 
                {-omega[2],      0.0f,  omega[0], 0.0f},
                { omega[1], -omega[0],      0.0f, 0.0f},
                {     0.0f,      0.0f,      0.0f, 0.0f}
            };

            Mat44 tensorSqr = tensor * tensor;

            Mat44 t1 = Mat44(kIdentityTag);
            Mat44 t2 = (std::sinf(theta) * tensor);
            Mat44 t3 = (1 - std::cosf(theta)) * tensorSqr;
            Mat44 res = t1 + t2 + t3;
            return res;
        }
    }
}