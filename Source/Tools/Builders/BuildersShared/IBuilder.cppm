module;

#include <filesystem>
#include <span>
#include <string_view>

export module sj.builders:IBuilder;
import sj.datadefs;

export namespace sj::build
{
class BuildContext;

class IBuilder
{
public:
    virtual ~IBuilder() = default;

    [[nodiscard]] virtual std::span<const std::string_view> GetExtensions() const = 0;
    [[nodiscard]] virtual std::string_view GetBuilderName() const = 0;
    [[nodiscard]] virtual std::string_view GetOutputExtension() const = 0;

    [[nodiscard]] virtual bool BuildItem(BuildContext& in_ctx,
                                         const std::filesystem::path& item) = 0;
};
} // namespace sj::build