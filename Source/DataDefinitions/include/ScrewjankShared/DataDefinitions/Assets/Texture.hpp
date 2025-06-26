#pragma once

// Shared Includes
#include <ScrewjankShared/DataDefinitions/Assets/AssetType.hpp>

// STD Includes
#include <cstdint>

namespace sj
{
    struct TextureHeader
    {
        AssetType type = AssetType::kTexture;
        int32_t width;
        int32_t height;
    };
}