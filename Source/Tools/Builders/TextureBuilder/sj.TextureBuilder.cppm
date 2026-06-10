module;

// SJ Headers
#include <ScrewjankStd/Assert.hpp>

// Library Includes
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// STD Includes
#include <span>
#include <filesystem>
#include <cstdio>
#include <fstream>
#include <span>

export module sj.TextureBuilder;
import sj.datadefs.assets.AssetType;
import sj.datadefs.assets.Texture;
import sj.builders;

export namespace sj::build
{
class TextureBuilder final : public IBuilder
{
public:
    [[nodiscard]] std::span<const std::string_view> GetExtensions() const override
    {
        using namespace std::literals;
        static constexpr std::array extensions = {".png"sv, ".jpg"sv};
        return extensions;
    }

    [[nodiscard]] std::string_view GetBuilderName() const override
    {
        return "Texture Builder";
    }

    [[nodiscard]] std::string_view GetOutputExtension() const override
    {
        return ".sj_tex";
    }

    bool BuildItem(const std::filesystem::path& item,
                   const std::filesystem::path& output_path) override
    {
        int texWidth = 0;
        int texHeight = 0;
        int texChannels = 0;
        stbi_set_flip_vertically_on_load(true);
        stbi_uc* pixels =
            stbi_load(item.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        std::streamsize imageBytes =
            texWidth * texHeight * STBI_rgb_alpha; // 4 bytes per pixel with STBI_rgb_alpha
        SJ_ASSERT(imageBytes > 0, "invalid image size");
        TextureHeader texture = {.asset_type = AssetType::kTexture,
                                 .bytesPerPixel = STBI_rgb_alpha,
                                 .width = texWidth,
                                 .height = texHeight};

        std::ofstream outputFile;
        outputFile.open(output_path, std::ios::out | std::ios::binary);
        SJ_ASSERT(outputFile.is_open(), "Failed to open output file {}", output_path.c_str());

        outputFile.write(reinterpret_cast<char*>(&texture), sizeof(texture));
        outputFile.write(reinterpret_cast<char*>(pixels), imageBytes);
        outputFile.close();

        stbi_image_free(pixels);

        return true;
    }
};
} // namespace sj::build
