module;

// Shared Includes
#include <ScrewjankShared/Math/Mat44.hpp>
#include <ScrewjankShared/string/StringHash.hpp>

export module sj.shared.datadefs:TransformComponent;

export namespace sj
{
    struct TransformComponent
    {
        SJ_STRUCT_TYPE_ID(TransformComponent);

        Mat44 localToParent;
    };
}