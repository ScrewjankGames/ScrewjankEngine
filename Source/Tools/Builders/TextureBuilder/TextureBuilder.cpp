// Shared Includes
#include <ScrewjankShared/DataDefinitions/Assets/Texture.hpp>
#include <ScrewjankShared/io/File.hpp>

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

    TextureHeader texture {AssetType::kTexture, texWidth, texHeight};

    File outputFile;
    outputFile.Open(outputFilePath, File::OpenMode::kWriteBinary);

    outputFile.WriteStruct(texture);
    outputFile.Write(pixels, imageBytes);
    outputFile.Close();

    stbi_image_free(pixels);

    return 0;
}