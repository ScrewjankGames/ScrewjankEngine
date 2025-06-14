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
            
            auto components = registry.GetComponents<CameraComponent>();
            SJ_ASSERT(components.size() > 0, "Scene has no camera component");
            
            for(const auto& [goId, cameraComponent] : components )
            {
                // TODO: What if there's multiple
                //GameObject* parentGo = scene->GetGameObject(component.ownerGameobjectId);
                //SJ_ASSERT(parentGo, "Failed to find parent game object!");

                Mat44 localToGoTransform = cameraComponent.localToGoTransform;
              
                const TransformComponent* goTransform = registry.GetComponent<TransformComponent>(goId);
                (void)goTransform;

                //const Mat44& goWorldSpaceTransform = parentGo->GetWorldSpaceTransform();
                
                Mat44 outputTransform = localToGoTransform;// * goWorldSpaceTransform;
                
                m_outputCameraMatrix = outputTransform;
                m_outputCameraMatrix.SetRow<3>(Vec4(-1, 1, 5, 1));
                break;
            }
        }

        [[nodiscard]] Mat44 GetOutputCameraMatrix() const
        {
            return m_outputCameraMatrix;
        }

    private:
        Mat44 m_outputCameraMatrix = Mat44(kIdentityTag);
    };
} // namespace sj