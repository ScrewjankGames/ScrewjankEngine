module;
#include <cstdint>

export module sj.datadefs.assets.Texture;
export import sj.datadefs.assets.AssetType;

export namespace sj
{
    struct TextureHeader
    {
        AssetType asset_type = AssetType::kTexture;
        uint8_t bytesPerPixel = 0;
        int32_t width = 0;
        int32_t height = 0;

        // Bytes of texture follow this struct in file
    };
}