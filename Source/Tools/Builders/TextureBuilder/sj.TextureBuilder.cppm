module;

// SJ Headers
#include <ScrewjankStd/Assert.hpp>

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

    bool BuildItem(BuildContext& in_ctx, const std::filesystem::path& item) override
    {
        auto&& [output_path, id] =
            in_ctx.Import(item, item.filename().replace_extension(".sj_tex"));

        BuildTextureFromFile(item, output_path);

        return true;
    }
};
} // namespace sj::build
