// Parent Include
#include <ScrewjankEngine/core/systems/CameraSystem.hpp>

// Engine Incudes
#include <ScrewjankEngine/core/systems/InputSystem.hpp>
#include <ScrewjankEngine/core/Scene.hpp>

// Shared Includes
#include <ScrewjankShared/utils/Assert.hpp>
#include <ScrewjankShared/Math/Vec2.hpp>
#include <ScrewjankShared/Math/Helpers.hpp>
#include <ScrewjankShared/DataDefinitions/Components/CameraComponent.hpp>

// STD Includes
#include <cmath>
#include <span>

namespace sj
{
    void CameraSystem::Process(Scene* scene, float deltaTime)
    {
        std::span<CameraComponent> components = scene->GetCameraComponents();
        SJ_ASSERT(components.size() > 0, "Scene has no camera component");

        // TODO: What if there's multiple
        CameraComponent& component = components[0];

        GameObject* parentGo = scene->GetGameObject(component.ownerGameobjectId);
        SJ_ASSERT(parentGo, "Failed to find parent game object!");

        Mat44 localToGoTransform = component.localToGoTransform;
        const Mat44& goWorldSpaceTransform = parentGo->GetWorldSpaceTransform();

        Mat44 outputTransform = localToGoTransform * goWorldSpaceTransform;

        m_outputCameraMatrix = outputTransform;

        //const Vec2& rightStick = InputSystem::GetRightStick();
        //constexpr float yawSpeed = ToRadians(45.0f);
        //const float deltaYaw = -rightStick.GetX() * yawSpeed * deltaTime;

        //constexpr float pitchSpeed = ToRadians(45.0f);
        //const float deltaPitch = -rightStick.GetY() * pitchSpeed * deltaTime;
        //m_eulerAngles[0] += deltaPitch;
        //m_eulerAngles[1] += deltaYaw;

        //m_outputCameraMatrix = FromEulerXYZ(m_eulerAngles, m_outputCameraMatrix[3]);

        //const Vec2& leftStick = InputSystem::GetLeftStick();
        //constexpr float linearSpeed = 4.0f;

        //const Vec4 forwardAxis = -m_outputCameraMatrix.GetZ();
        //const Vec4& rightAxis = m_outputCameraMatrix.GetX();

        //Vec4 deltaPosition = rightAxis * leftStick.GetX() + forwardAxis * -leftStick.GetY();
        //deltaPosition *= deltaTime * linearSpeed;
        //m_outputCameraMatrix[3] += deltaPosition;
    }

    Mat44 CameraSystem::GetOutputCameraMatrix() const
    {
        return m_outputCameraMatrix;
    }

} // namespace sj