module;

#include <ScrewjankStd/Assert.hpp>

#include <span>
#include <filesystem>

export module sj.SceneBuilder;
import sj.builders;

export namespace sj::build
{
class SceneBuilder final : public IBuilder
{
public:
    [[nodiscard]] std::span<const std::string_view> GetExtensions() const override
    {
        using namespace std::literals;
        static constexpr std::array extensions = {".scene"sv};
        return extensions;
    }

    [[nodiscard]] std::string_view GetBuilderName() const override
    {
        return "Scene Builder";
    }

    bool BuildItem(BuildContext& in_ctx, const std::filesystem::path& item) override
    {
        auto&& [output_path, id] =
            in_ctx.Import(item, item.filename().replace_extension(".sj_scene"));

        bool success =
            std::filesystem::copy_file(item,
                                       output_path,
                                       std::filesystem::copy_options::overwrite_existing);
        return success;
    }
};
} // namespace sj::build
