#pragma once

// Shared Includes
#include <ScrewjankShared/Math/Mat44.hpp>
#include <ScrewjankShared/string/StringHash.hpp>
#include <ScrewjankShared/DataDefinitions/GameObjectPrototype.hpp>

namespace sj
{
    struct TransformComponent
    {
        SJ_STRUCT_TYPE_ID(TransformComponent);

        // Static Data
        GameObjectId ownerGameobjectId;
        Mat44 localToParent;

        // Runtime Data
        TransformComponent* parentTransform;
    };
}