// Parent Include
#include <ScrewjankEngine/core/systems/CameraSystem.hpp>

// Engine Incudes
#include <ScrewjankEngine/core/systems/InputSystem.hpp>

// Shared Includes
#include <ScrewjankShared/utils/Assert.hpp>
#include <ScrewjankShared/Math/Vec2.hpp>
#include <ScrewjankShared/Math/Helpers.hpp>

// STD Includes
#include <cmath>
#include <span>

import sj.shared.datadefs;

namespace sj
{
    void CameraSystem::Process(Scene* scene, float deltaTime)
    {
        // std::span<CameraComponent> components = scene->GetCameraComponents();
        // SJ_ASSERT(components.size() > 0, "Scene has no camera component");

        // // TODO: What if there's multiple
        // CameraComponent& component = components[0];

        // //GameObject* parentGo = scene->GetGameObject(component.ownerGameobjectId);
        // //SJ_ASSERT(parentGo, "Failed to find parent game object!");

        // Mat44 localToGoTransform = component.localToGoTransform;
        // //const Mat44& goWorldSpaceTransform = parentGo->GetWorldSpaceTransform();

        // Mat44 outputTransform = localToGoTransform;// * goWorldSpaceTransform;

        // m_outputCameraMatrix = outputTransform;
    }

    Mat44 CameraSystem::GetOutputCameraMatrix() const
    {
        return m_outputCameraMatrix;
    }

} // namespace sj