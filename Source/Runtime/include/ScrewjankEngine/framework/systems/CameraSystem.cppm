module;
#include <ScrewjankStd/Assert.hpp>

export module sj.engine.framework.systems.CameraSystem;
import sj.std.math;
import sj.shared.datadefs;
import sj.engine.framework.ecs;

export namespace sj
{
    class CameraSystem
    {
    public:
        void Process(ECSRegistry& registry, float deltaTime)
        {
            (void)deltaTime;
            
            std::span<CameraComponent> components = registry.GetComponents<CameraComponent>();
            SJ_ASSERT(components.size() > 0, "Scene has no camera component");
            
            // TODO: What if there's multiple
            CameraComponent& component = components[0];
            
            //GameObject* parentGo = scene->GetGameObject(component.ownerGameobjectId);
            //SJ_ASSERT(parentGo, "Failed to find parent game object!");
            
            Mat44 localToGoTransform = component.localToGoTransform;
            //const Mat44& goWorldSpaceTransform = parentGo->GetWorldSpaceTransform();
            
            Mat44 outputTransform = localToGoTransform;// * goWorldSpaceTransform;
            
            m_outputCameraMatrix = outputTransform;
        }

        Mat44 GetOutputCameraMatrix() const
        {
            return m_outputCameraMatrix;
        }

    private:
        Mat44 m_outputCameraMatrix = Mat44(kIdentityTag);
    };
} // namespace sj