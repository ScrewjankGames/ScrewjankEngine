module;

// Shared Includes
#include <ScrewjankShared/Math/Mat44.hpp>
#include <ScrewjankShared/string/StringHash.hpp>

export module sj.shared.datadefs:CameraComponent;

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