module;

#include <string>
#include <filesystem>
#include <string_view>

export module sj.builders:Utils;

export namespace sj::build::utils
{

std::filesystem::path GetInstallFileName(std::filesystem::path path,
                                         std::string_view output_extension)
{
    std::filesystem::path outputPath = path.stem();
    outputPath = outputPath.replace_extension(output_extension);

    return outputPath;
}

std::filesystem::path GetImportPath(std::filesystem::path path)
{
    path.replace_extension(path.extension().string() + ".import");
    return path;
}

} // namespace sj::build::utils