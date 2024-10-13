#pragma once

// Shared Includes
#include <ScrewjankShared/Math/Mat44.hpp>
#include <ScrewjankShared/string/StringHash.hpp>
#include <ScrewjankShared/DataDefinitions/GameObjectPrototype.hpp>

namespace sj
{
    struct CameraComponent
    {
        static constexpr uint32_t kTypeId = StringHash("CameraComponent").AsInt();

        GameObjectId ownerGameobjectId;

        Mat44 localToGoTransform;
        float fov;
        float nearPlane;
        float farPlane;
    };
}