#pragma once

import sj.engine.framework;
import sj.shared.math;

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