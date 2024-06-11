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
          m_translation {0, 0, 2.0f, 1.0f}

    {
    }

    void CameraSystem::Process(float deltaTime)
    {
        Vec4 deltaPosition;
        {
            const Vec2& leftStick = InputSystem::GetLeftStick();
            constexpr float linearSpeed = 1.0f;
            deltaPosition = Vec4(leftStick[0], 0, leftStick[1], 0) * linearSpeed * deltaTime;
        }

        const Vec2& rightStick = InputSystem::GetRightStick();
        constexpr float yawSpeed = ToRadians(45.0f);
        const float deltaYaw = -rightStick.GetX() * yawSpeed * deltaTime;

        constexpr float pitchSpeed = ToRadians(45.0f);
        const float deltaPitch = -rightStick.GetY() * pitchSpeed * deltaTime;

        m_eulerAngles[0] += deltaPitch;
        m_eulerAngles[1] += deltaYaw;

        m_translation += deltaPosition;
        //m_outputCameraMatrix = m_outputCameraMatrix * deltaYaw * deltaPitch;
    }

    Mat44 CameraSystem::GetOutputCameraMatrix() const
    {
        Mat44 output = FromEulerXYZ(m_eulerAngles);
        output[3] = m_translation;

        return output;
    }

} // namespace sj