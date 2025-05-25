module;

// Shared Includes
#include <ScrewjankShared/string/StringHash.hpp>

export module sj.shared.datadefs:CameraComponent;
import sj.shared.math;

export namespace sj
{
    struct CameraComponent
    {
        SJ_STRUCT_TYPE_ID(CameraComponent);

        Mat44 localToGoTransform;
        float fov;
        float nearPlane;
        float farPlane;
    };
}