#pragma once
// Shared Includes
#include <ScrewjankShared/Math/Mat44.hpp>

namespace sj
{
    class CameraSystem
    {
    public:
        CameraSystem();

        void Process(float deltaTime);
        const Mat44& GetOutputCameraMatrix() const;

    private:
        Mat44 m_outputCameraMatrix;
    };
}