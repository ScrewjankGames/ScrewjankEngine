// Shared Includes
#include <ScrewjankShared/DataDefinitions/Texture.hpp>
#include <ScrewjankShared/IO/File.hpp>

// Library Includes
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// STD Includes
#include <new>
#include <cstdio>

using namespace sj;

int main(int argc, char** argv)
{
    const char* inputFilePath = argv[1];
    const char* outputFilePath = argv[2];

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(inputFilePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    size_t imageBytes = texWidth * texHeight * 4; // 4 bytes per pixel with STBI_rgb_alpha

    void* textureMem = malloc(sizeof(Texture) + imageBytes);

    Texture& texture = *new(textureMem) Texture {AssetType::kTexture, texWidth, texHeight};
    memcpy(texture.data, pixels, imageBytes);

    File outputFile;
    outputFile.Open(outputFilePath, File::OpenMode::kWriteBinary);

    outputFile.Write(&texture.type, sizeof(Texture::type));
    outputFile.Write(&texture.width, sizeof(Texture::width));
    outputFile.Write(&texture.height, sizeof(Texture::height));
    outputFile.Write(&texture.data, imageBytes);
    outputFile.Close();

    stbi_image_free(pixels);

    return 0;
}