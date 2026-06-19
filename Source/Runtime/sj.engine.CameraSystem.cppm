module;
#include <ScrewjankStd/Assert.hpp>

export module sj.engine.CameraSystem;
import sj.std;

import sj.engine.CameraComponent;
import sj.engine.TransformComponent;
import sj.engine.ecs.ECSRegistry;

export namespace sj
{
class CameraSystem
{
public:
    static constexpr type_list<CameraComponent> kOwnedComponents;

    CameraSystem() = default;

    void Process(ECSRegistry& registry, [[maybe_unused]] float deltaTime)
    {
        auto components = registry.GetComponents<CameraComponent>();

        for(const auto& [goId, cameraComponent] : components)
        {
            // TODO: What if there's multiple
            Mat44 localToGoTransform = cameraComponent.localToGoTransform;
            const TransformComponent* goTransform = registry.GetComponent<TransformComponent>(goId);
            const Mat44& goWorldSpaceTransform = goTransform->localToParent;

            Mat44 outputTransform = localToGoTransform * goWorldSpaceTransform;

            m_outputCameraMatrix = outputTransform;

            return;
        }
        
        SJ_ASSERT(false, "Scene has no camera component");
    }

    [[nodiscard]] Mat44 GetOutputCameraMatrix() const
    {
        return m_outputCameraMatrix;
    }

private:
    Mat44 m_outputCameraMatrix = Mat44(kIdentityTag);
};
} // namespace sj