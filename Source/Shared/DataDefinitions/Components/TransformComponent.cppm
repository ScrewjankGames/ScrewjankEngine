module;

// Shared Includes
#include <ScrewjankStd/TypeMacros.hpp>

export module sj.shared.datadefs:TransformComponent;
import sj.std.math;
import sj.std.string_hash;

export namespace sj
{
    struct TransformComponentChunk
    {
        SJ_STRUCT_TYPE_ID(TransformComponentChunk);

        string_hash parentId;
        Mat44 localToParent;
    };

    struct TransformComponent
    {
        SJ_STRUCT_TYPE_ID(TransformComponent);
        Mat44 localToParent;

        [[nodiscard]] auto GetWorldSpaceTransform() const -> const Mat44&
        {
            // TODO: Hierarchies
            return localToParent;
        }
    };
} // namespace sj