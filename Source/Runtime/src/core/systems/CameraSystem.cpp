// Parent Include
#include <ScrewjankEngine/core/systems/CameraSystem.hpp>

// Engine Incudes
#include <ScrewjankEngine/core/systems/InputSystem.hpp>
#include <ScrewjankEngine/utils/Assert.hpp>

// Shared Includes
#include <ScrewjankShared/Math/Helpers.hpp>

// STD Includes
#include <cmath>

namespace sj
{

    CameraSystem::CameraSystem() 
        : m_eulerAngles {0.0f, 0.0f, 0.0f},
          m_outputCameraMatrix {kIdentityTag}
    {
        m_outputCameraMatrix[3] = Vec4 {0.0f, 0.0f, 2.0f, 1.0f};
    }

    void CameraSystem::Process(float deltaTime)
    {
        const Vec2& rightStick = InputSystem::GetRightStick();
        constexpr float yawSpeed = ToRadians(45.0f);
        const float deltaYaw = -rightStick.GetX() * yawSpeed * deltaTime;

        constexpr float pitchSpeed = ToRadians(45.0f);
        const float deltaPitch = -rightStick.GetY() * pitchSpeed * deltaTime;
        m_eulerAngles[0] += deltaPitch;
        m_eulerAngles[1] += deltaYaw;

        m_outputCameraMatrix = FromEulerXYZ(m_eulerAngles, m_outputCameraMatrix[3]);

        const Vec2& leftStick = InputSystem::GetLeftStick();
        constexpr float linearSpeed = 1.0f;

        const Vec4 forwardAxis = -m_outputCameraMatrix.GetZ();
        const Vec4& rightAxis = m_outputCameraMatrix.GetX();

        if(MagnitudeSqr(leftStick) > 0.01f)
        {
            Vec4 deltaPosition =
                Normalize(rightAxis * leftStick.GetX() + forwardAxis * -leftStick.GetY());
            deltaPosition *= deltaTime * linearSpeed;
            m_outputCameraMatrix[3] += deltaPosition;
        }
    }

    Mat44 CameraSystem::GetOutputCameraMatrix() const
    {
        return m_outputCameraMatrix;
    }

} // namespace sj