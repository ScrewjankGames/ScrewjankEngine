#pragma once

// STD Includes
#include <cstdint>

namespace sj
{
    enum class AssetType : uint8_t
    {
        kInvalid,
        kTexture,
        kMesh
    };
}