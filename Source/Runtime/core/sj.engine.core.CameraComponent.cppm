module;
#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankEngine/components/ComponentMacros.hpp>

export module sj.engine.core.CameraComponent;
import sj.std.math;
import sj.datadefs.components.CameraChunk;
import sj.engine.core.TransformComponent;

export namespace sj
{
    struct CameraComponent
    {
        SJ_COMPONENT(CameraComponent, CameraChunk);

        Mat44 localToGoTransform;
        float fov = 0;
        float nearPlane = 0;
        float farPlane = 0;
    };
}