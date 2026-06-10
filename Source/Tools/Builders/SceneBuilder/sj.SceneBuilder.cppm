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

    [[nodiscard]] std::string_view GetOutputExtension() const override
    {
        return ".sj_scene";
    }

    bool BuildItem(const std::filesystem::path& item,
                   const std::filesystem::path& output_path) override
    {

        return std::filesystem::copy_file(item,
                                          output_path,
                                          std::filesystem::copy_options::overwrite_existing);
    }
};
} // namespace sj::build
