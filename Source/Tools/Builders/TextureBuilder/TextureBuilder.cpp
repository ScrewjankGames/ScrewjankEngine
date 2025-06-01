// Parent Include
#include "TextureBuilder.hpp"

// Shared Includes
#include <ScrewjankShared/DataDefinitions/Assets/Texture.hpp>
#include <ScrewjankShared/io/File.hpp>
#include <ScrewjankStd/Assert.hpp>

// Library Includes
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// STD Includes
#include <cstdio>

namespace sj::build 
{

bool TextureBuilder::BuildItem(const std::filesystem::path& item, const std::filesystem::path& output_path) const
{
    int texWidth = 0;
    int texHeight = 0;
    int texChannels = 0;
    stbi_uc* pixels = stbi_load(item.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    int64_t imageBytes = texWidth * texHeight * 4; // 4 bytes per pixel with STBI_rgb_alpha
    SJ_ASSERT(imageBytes > 0, "invalid image size");
    TextureHeader texture {.type=AssetType::kTexture, .width=texWidth, .height=texHeight};

    File outputFile;
    outputFile.Open(output_path.c_str(), File::OpenMode::kWriteBinary);

    outputFile.WriteStruct(texture);
    outputFile.Write(pixels, static_cast<size_t>(imageBytes));
    outputFile.Close();

    stbi_image_free(pixels);

    return true;
}

}