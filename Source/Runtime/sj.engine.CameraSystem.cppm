module;
#include <ScrewjankStd/Assert.hpp>

export module sj.engine.CameraSystem;
import sj.std;

import sj.engine.CameraComponent;
import sj.engine.TransformComponent;
import sj.engine.ecs;

export namespace sj
{
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
            // TODO: What if there's multiple
            Mat44 localToGoTransform = camera.localToGoTransform;
            const Mat44& goWorldSpaceTransform = transform.localToParent;

            Mat44 outputTransform = localToGoTransform * goWorldSpaceTransform;

            mOutputCameraGo = goId;
            mOutputCameraLW = outputTransform;

            return;
        }
        
        SJ_ASSERT(false, "Scene has no camera component");
    }

    [[nodiscard]] Mat44 GetOutputCameraMatrix() const
    {
        return mOutputCameraLW;
    }

private:
    GameObjectId mOutputCameraGo {};
    Mat44 mOutputCameraLW = Mat44(kIdentityTag);
};
} // namespace sj