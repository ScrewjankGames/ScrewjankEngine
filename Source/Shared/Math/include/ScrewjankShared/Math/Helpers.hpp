#pragma once

// Screwjank Includes
#include <ScrewjankShared/Math/Vec4.hpp>
#include <ScrewjankShared/Math/Mat44.hpp>

// STD Includes
#include <numbers>

namespace sj
{
    inline constexpr float ToRadians(float degrees) { return static_cast<float>(degrees * std::numbers::pi / 180); }

    /**
     * @return Create a view matrix that looks at the given target.
     * 
     * In terms of cameras, calculates the inverse of the camera matrix
     */
    Mat44 LookAt(const Vec4& eye, const Vec4& target, const Vec4& up);

    /**
     * Computes perpsective projection matrix
     * @param verticalFOV: Vertical FOV of the view frustrum
     * @param aspectRatio: Render surface width / height
     * @param near: Near render plane
     * @param far: Far render plane
     */
    Mat44 PerspectiveProjection(float verticalFOV, float aspectRatio, float near, float far);

}