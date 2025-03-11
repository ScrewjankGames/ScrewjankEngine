#pragma once

// Shared Includes
#include <ScrewjankShared/string/StringHash.hpp>

// STD Includes
#include <cstdint>
#include "ScrewjankShared/DataDefinitions/GameObjectPrototype.hpp"

#include <vector>
#include <any>
#include <map>
#include <string>

namespace sj
{
    struct ScenePrototype
    {
        StringHash name; 
        uint32_t memory; // Free ram allocated to this scene
        uint32_t scriptPoolSize; // Number of elements needed in the script pool
        uint32_t numGameObjects; 
        uint32_t numComponentLists;
    };

    struct ComponentListHeader
    {
        uint32_t componentTypeId;
        uint32_t numComponents;
    };
}