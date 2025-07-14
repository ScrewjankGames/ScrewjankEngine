module;
#include <cstddef>

// SJ Includes
#include <ScrewjankDataDefinitions/Assets/AssetType.hpp>
#include <ScrewjankDataDefinitions/ChunkMacros.hpp>

// STD Includes
#include <cstdint>

export module sj.datadefs.assets.Texture;

export namespace sj
{
    struct TextureHeader
    {
        AssetType asset_type = AssetType::kTexture;
        int32_t width = 0;
        int32_t height = 0;

        // Bytes of texture follow this struct in file
    };
}