// Parent Include
#include "TextureBuilder.hpp"

// Datadefs
#include <ScrewjankDataDefinitions/Assets/Texture.hpp>

// SJSTD
#include <ScrewjankStd/Assert.hpp>

// Library Includes
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// STD Includes
#include <cstdio>
#include <fstream>

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

    std::ofstream outputFile;
    outputFile.open(output_path, std::ios::out | std::ios::binary);
    SJ_ASSERT(outputFile.is_open(), "Failed to open output file {}", output_path.c_str());

    outputFile.write(reinterpret_cast<char*>(&texture), sizeof(texture));
    outputFile.write(reinterpret_cast<char*>(pixels), static_cast<size_t>(imageBytes));
    outputFile.close();

    stbi_image_free(pixels);

    return true;
}

}