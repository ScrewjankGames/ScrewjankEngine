#pragma once

// Shared Includes
#include <ScrewjankShared/Math/Mat44.hpp>
#include <ScrewjankShared/Math/Vec3.hpp>

import sj.engine.framework;

namespace sj
{
    class CameraSystem
    {
    public:
        void Process(Scene* scene, float deltaTime);

        Mat44 GetOutputCameraMatrix() const;

    private:
        Mat44 m_outputCameraMatrix = Mat44(kIdentityTag);
    };
}