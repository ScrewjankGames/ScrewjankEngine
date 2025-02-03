// Parent Include
#include "TextureBuilder.hpp"

// Shared Includes
#include <ScrewjankShared/DataDefinitions/Assets/Texture.hpp>
#include <ScrewjankShared/io/File.hpp>

// Library Includes
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// STD Includes
#include <cstdio>

namespace sj::build 
{

bool TextureBuilder::BuildItem(const std::filesystem::path& item, const std::filesystem::path& output_path) const
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(item.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    size_t imageBytes = texWidth * texHeight * 4; // 4 bytes per pixel with STBI_rgb_alpha

    TextureHeader texture {AssetType::kTexture, texWidth, texHeight};

    File outputFile;
    outputFile.Open(output_path.c_str(), File::OpenMode::kWriteBinary);

    outputFile.WriteStruct(texture);
    outputFile.Write(pixels, imageBytes);
    outputFile.Close();

    stbi_image_free(pixels);

    return true;
}

}