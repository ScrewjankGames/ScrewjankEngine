module;

#include <ScrewjankStd/Assert.hpp>

#include <span>
#include <filesystem>

export module sj.SceneBuilder;
import sj.build.IGlobBuilder;

export namespace sj::build
{
class SceneBuilder final : public IGlobBuilder
{
public:
    [[nodiscard]] std::span<const char* const> GetExtensions() const override
    {
        static constexpr std::array extensions = {".scene"};
        return extensions;
    }

    [[nodiscard]] const char* GetBuilderName() const override
    {
        return "Scene Builder";
    }

    [[nodiscard]] const char* GetOutputExtension() const override
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
