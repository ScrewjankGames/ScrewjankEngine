#pragma once

// Shared Includes
#include <ScrewjankShared/Math/Mat44.hpp>
#include <ScrewjankShared/Math/Vec3.hpp>

namespace sj
{
    class CameraSystem
    {
    public:
        CameraSystem();

        void Process(float deltaTime);
        Mat44 GetOutputCameraMatrix() const;

    private:
        Vec3 m_eulerAngles;
        Vec4 m_translation;
    };
}