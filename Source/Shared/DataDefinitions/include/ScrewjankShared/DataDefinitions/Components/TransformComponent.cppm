module;

// Shared Includes
#include <ScrewjankShared/string/StringHash.hpp>

export module sj.shared.datadefs:TransformComponent;
import sj.std.math;

export namespace sj
{
    struct TransformComponent
    {
        SJ_STRUCT_TYPE_ID(TransformComponent);
        Mat44 localToParent;
    };
}