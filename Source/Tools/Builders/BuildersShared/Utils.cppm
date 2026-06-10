module;

#include <string>
#include <filesystem>
#include <string_view>

export module sj.builders:Utils;

export namespace sj::build::utils
{

std::filesystem::path GetDestinationPath(std::filesystem::path item,
                                         std::filesystem::path input_dir,
                                         std::filesystem::path output_dir,
                                         std::string_view output_extension)
{
    std::filesystem::path relativePath = std::filesystem::relative(item, input_dir);
    std::filesystem::path outputPath = output_dir / relativePath;
    outputPath = outputPath.replace_extension(output_extension);

    return outputPath;
}

std::filesystem::path GetImportPath(std::filesystem::path path)
{
    path.replace_extension(path.extension().string() + ".import");
    return path;
}

std::filesystem::path GetAssetPath(std::filesystem::path importPath)
{
    using namespace std::literals;
    auto extension = [&] -> std::string_view {
        std::string fullExtension = importPath.extension().string();

        std::string_view res(fullExtension);
        res.remove_suffix(".import"sv.size());
        return res;
    }();
    return extension;
}
} // namespace sj::build::utils