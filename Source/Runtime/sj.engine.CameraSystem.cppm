module;
#include <ScrewjankStd/Assert.hpp>

export module sj.engine.CameraSystem;
import sj.std;

import sj.engine.TransformComponent;
import sj.engine.ecs;

export namespace sj
{
    
struct CameraComponent
{
    mat44 localToGoTransform;
    float fov = 0;
    float nearPlane = 0;
    float farPlane = 0;
};

class CameraSystem
{
public:
    static constexpr type_list<CameraComponent> kRegisteredComponents;

    CameraSystem() = default;

    void Process(ECSRegistry& registry, [[maybe_unused]] float deltaTime)
    {
        auto query = registry.QueryWithIds<TransformComponent, CameraComponent>();

        for(auto&& [goId, transform, camera] : query)
        {
            GameObject cameraGo(goId, registry);
            const mat44& goWorldSpaceTransform = cameraGo.GetTransformLW();

            // TODO: What if there's multiple
            mat44 localToGoTransform = camera.localToGoTransform;

            mat44 outputTransform = localToGoTransform * goWorldSpaceTransform;

            mOutputCameraGo = goId;
            mOutputCameraLW = outputTransform;

            return;
        }

        SJ_ASSERT(false, "Scene has no camera component");
    }

    [[nodiscard]] mat44 GetOutputCameraMatrix() const
    {
        return mOutputCameraLW;
    }

private:
    GameObjectId mOutputCameraGo {};
    mat44 mOutputCameraLW = mat44(kIdentityTag);
};
} // namespace sj