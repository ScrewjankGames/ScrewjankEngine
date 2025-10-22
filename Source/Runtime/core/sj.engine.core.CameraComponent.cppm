module;
#include <ScrewjankStd/Assert.hpp>

export module sj.engine.core.CameraComponent;
import sj.std.math;
import sj.engine.core.TransformComponent;

export namespace sj
{
    struct CameraComponent
    {
        Mat44 localToGoTransform;
        float fov = 0;
        float nearPlane = 0;
        float farPlane = 0;
    };
}