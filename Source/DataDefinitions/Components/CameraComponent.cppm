module;

// Shared Includes
#include <ScrewjankStd/TypeMacros.hpp>

export module sj.datadefs:CameraComponent;
import sj.std.math;

export namespace sj
{
    struct CameraComponent
    {
        SJ_STRUCT_TYPE_ID(CameraComponent);

        Mat44 localToGoTransform;
        float fov = 0;
        float nearPlane = 0;
        float farPlane = 0;
    };
}