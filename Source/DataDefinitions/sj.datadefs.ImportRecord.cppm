module;

#include <ScrewjankStd/Assert.hpp>

#include <glaze/glaze.hpp>

#include <filesystem>
#include <flat_map>
#include <string_view>

export module sj.datadefs.ImportRecord;
import sj.datadefs.AssetDB;

export namespace sj
{
struct ImportRecord
{
    static ImportRecord Load(std::string_view importPath)
    {
        ImportRecord record;
        glz::error_ctx ctx = glz::read_file_json(record, importPath, std::vector<char> {});
        SJ_ASSERT(ctx.ec == glz::error_code::none,
                  "Failed to load import record from path {}",
                  importPath);

        return record;
    }

    // Install filename to AssetID
    std::flat_map<std::filesystem::path, AssetID> installed_assets;
};
} // namespace sj