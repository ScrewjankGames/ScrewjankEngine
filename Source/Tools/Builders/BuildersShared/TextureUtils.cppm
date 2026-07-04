module;

#include <ScrewjankStd/Assert.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <filesystem>
#include <fstream>
#include <ios>
#include <span>

export module sj.builders:TextureUtils;
import sj.datadefs;

namespace sj::build
{
void WriteTextureToFile(const std::filesystem::path& outputFilePath,
                        int texWidth,
                        int texHeight,
                        int texChannels,
                        stbi_uc* pixels)
{
    std::streamsize imageBytes =
        texWidth * texHeight * STBI_rgb_alpha; // 4 bytes per pixel with STBI_rgb_alpha
    SJ_ASSERT(imageBytes > 0, "invalid image size");
    TextureHeader texture = {.asset_type = AssetType::kTexture,
                             .bytesPerPixel = STBI_rgb_alpha,
                             .width = texWidth,
                             .height = texHeight};

    std::ofstream outputFile;
    outputFile.open(outputFilePath, std::ios::out | std::ios::binary);
    SJ_ASSERT(outputFile.is_open(), "Failed to open output file {}", outputFilePath.c_str());

    outputFile.write(reinterpret_cast<char*>(&texture), sizeof(texture));
    outputFile.write(reinterpret_cast<char*>(pixels), imageBytes);
    outputFile.close();
}

} // namespace sj::build

export namespace sj::build
{
void BuildTextureFromMemory(std::span<const std::byte> memory,
                            const std::filesystem::path& outputFilePath)
{
    int texWidth = 0;
    int texHeight = 0;
    int texChannels = 0;

    stbi_set_flip_vertically_on_load(true);
    stbi_uc* pixels = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(memory.data()),
                                            static_cast<int>(memory.size()),
                                            &texWidth,
                                            &texHeight,
                                            &texChannels,
                                            STBI_rgb_alpha);

    WriteTextureToFile(outputFilePath, texWidth, texHeight, texChannels, pixels);

    stbi_image_free(pixels);
}

void BuildTextureFromFile(const std::filesystem::path& inputFilePath,
                          const std::filesystem::path& outputFilePath)
{
    int texWidth = 0;
    int texHeight = 0;
    int texChannels = 0;
    stbi_set_flip_vertically_on_load(true);
    stbi_uc* pixels =
        stbi_load(inputFilePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    WriteTextureToFile(outputFilePath, texWidth, texHeight, texChannels, pixels);

    stbi_image_free(pixels);
}
} // namespace sj::build