// Parent Include
#include <ScrewjankShared/Math/Helpers.hpp>

// STD Includes
#include <cmath>

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

}