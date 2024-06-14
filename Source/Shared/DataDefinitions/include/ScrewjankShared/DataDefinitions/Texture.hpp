#pragma once

// Shared Includes
#include <ScrewjankShared/DataDefinitions/AssetType.hpp>

// STD Includes
#include <cstdint>

namespace sj
{
    struct Texture
    {
        AssetType type = AssetType::kTexture;
        int32_t width;
        int32_t height;
        uint8_t data[1]; // Bytes of image follow this struct
    };
}