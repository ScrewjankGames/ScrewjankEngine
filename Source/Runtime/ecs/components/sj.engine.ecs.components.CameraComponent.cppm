module;
#include <ScrewjankEngine/components/ComponentMacros.hpp>

export module sj.engine.ecs.components.CameraComponent;
import sj.engine.ecs;
import sj.datadefs;
import sj.std.math;

export namespace sj
{
    class CameraComponent
    {
    public:
        CameraComponent(const CameraChunk& chunk)
            : localToGoTransform(chunk.localToGoTransform),
              fov(chunk.fov),
              nearPlane(chunk.nearPlane),
              farPlane(chunk.farPlane)
        {
            // GameObjectId parentGoId = registry.FindGameObject(data.parentId);
        }

        SJ_COMPONENT(CameraComponent, CameraChunk);

        Mat44 localToGoTransform;
        float fov = 0;
        float nearPlane = 0;
        float farPlane = 0;
    };
} // namespace sj