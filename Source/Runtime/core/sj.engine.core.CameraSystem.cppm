module;
#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankEngine/components/ComponentMacros.hpp>

export module sj.engine.core.CameraSystem;
import sj.engine.core.CameraComponent;
import sj.engine.core.TransformComponent;

import sj.std.math;
import sj.engine.ecs.ECSRegistry;
import sj.datadefs.components.CameraChunk;

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
                Mat44 localToGoTransform = cameraComponent.localToGoTransform;
                const TransformComponent* goTransform = registry.GetComponent<TransformComponent>(goId);              
                const Mat44& goWorldSpaceTransform = goTransform->localToParentTransform;
                
                Mat44 outputTransform = localToGoTransform * goWorldSpaceTransform;
                
                m_outputCameraMatrix = outputTransform;

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