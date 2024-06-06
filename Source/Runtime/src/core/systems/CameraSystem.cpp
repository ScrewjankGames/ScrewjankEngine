// Parent Include
#include <ScrewjankEngine/core/systems/CameraSystem.hpp>

// Engine Incudes
#include <ScrewjankEngine/core/systems/InputSystem.hpp>

// Shared Includes
#include <ScrewjankShared/Math/Helpers.hpp>

namespace sj
{

    CameraSystem::CameraSystem() 
        : m_outputCameraMatrix(kIdentityTag)
    {
        m_outputCameraMatrix = AffineInverse(LookAt(Vec4(0, 0, 2.0f, 0), Vec4(), Vec4::UnitY));
    }

    void CameraSystem::Process(float deltaTime)
    {
        const Vec2& leftStick = InputSystem::GetLeftStick();
        constexpr float speed = 1.0f;

        Vec4 newCameraPos = m_outputCameraMatrix.GetW();
        newCameraPos += Vec4(leftStick[0], 0, leftStick[1], 0) * speed * deltaTime;

        m_outputCameraMatrix[3] = newCameraPos;
    }

    const Mat44& CameraSystem::GetOutputCameraMatrix() const
    {
        return m_outputCameraMatrix;
    }

} // namespace sj