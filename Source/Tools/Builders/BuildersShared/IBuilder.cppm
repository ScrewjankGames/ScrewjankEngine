module;

#include <filesystem>
#include <span>
#include <string_view>

export module sj.builders:IBuilder;

export namespace sj::build
{
class IBuilder
{
public:
    virtual ~IBuilder() = default;

    [[nodiscard]] virtual std::span<const std::string_view> GetExtensions() const = 0;
    [[nodiscard]] virtual std::string_view GetBuilderName() const = 0;
    [[nodiscard]] virtual std::string_view GetOutputExtension() const = 0;

    [[nodiscard]] virtual bool BuildItem(const std::filesystem::path& item,
                                         const std::filesystem::path& output_path) = 0;
};
} // namespace sj::build