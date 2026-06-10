#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/Log.hpp>

#include <glaze/glaze.hpp>

#include <print>
#include <filesystem>
#include <functional>
#include <flat_map>
#include <string_view>
#include <vector>
#include <ranges>
#include <tuple>

import sj.std;
import sj.builders;
import sj.datadefs;

import sj.TextureBuilder;
import sj.SceneBuilder;
import sj.MeshBuilder;

using namespace sj::build;
namespace fs = std::filesystem;

struct CommandLineArgs
{
    std::string_view projectDir;
    std::string_view assetDir;
    std::string_view installDir;
};

CommandLineArgs ParseCommandLineArgs(std::span<const char*> commandLine)
{
    CommandLineArgs args;

    auto cursor = commandLine.begin() + 1;
    auto tryConsumeFn = [&](std::string_view arg, std::string_view& out) -> bool {
        if(std::string_view(*cursor) == arg)
        {
            ++cursor;
            SJ_ASSERT(cursor != commandLine.end(),
                      "Failed to parse command line arg {}. Missing argument",
                      arg);

            out = *cursor;
            return true;
        }

        return false;
    };

    while(cursor != commandLine.end())
    {
        if(tryConsumeFn("--project-dir", args.projectDir))
            continue;

        if(tryConsumeFn("--asset-dir", args.assetDir))
            continue;

        if(tryConsumeFn("--data-install-dir", args.installDir))
            continue;

        ++cursor;
    }

    return args;
}

struct BuildRecord
{
    fs::path assetPath;
    fs::path importPath;
    fs::path installPath;
};

template <class... Builders>
class BuildContext
{
public:
    BuildContext(std::string_view projectDir)
        : mProjectDir(projectDir), mAssetDir(mProjectDir / "Assets"),
          mInstallDir(mProjectDir / "Data")
    {
        auto registerFn = [&](IBuilder& builder) {
            for(std::string_view extension : builder.GetExtensions())
                mBuilderDispatch[extension] = &builder;
        };

        std::apply(
            [&](auto&&... args) {
                (registerFn(std::forward<decltype(args)>(args)), ...);
            },
            mBuilders);

        for(const fs::directory_entry& entry : fs::recursive_directory_iterator(mAssetDir))
        {
            if(!entry.is_regular_file())
                continue;

            fs::path itemPath = entry.path();

            auto it = mBuilderDispatch.find(itemPath.extension());
            if(it == mBuilderDispatch.end())
                continue;

            auto&& [inputExtension, builder] = *it;

            std::vector<BuildRecord>& items = mBuildDB[inputExtension];

            fs::path installPath = utils::GetDestinationPath(itemPath,
                                                             mAssetDir,
                                                             mInstallDir,
                                                             builder->GetOutputExtension());

            items.emplace_back(BuildRecord {.assetPath = itemPath,
                                            .importPath = utils::GetImportPath(itemPath),
                                            .installPath = std::move(installPath)});
        }
    }

    IBuilder* GetBuilder(auto&& key)
    {
        auto it = mBuilderDispatch.find(key);
        if(it == mBuilderDispatch.end())
            return nullptr;

        return it->second;
    }

    [[nodiscard]] std::ranges::range auto GetAllItems() const
    {
        return mBuildDB.values() | std::views::join;
    }

    [[nodiscard]] const std::flat_map<fs::path, std::vector<BuildRecord>>& GetBuildDB() const
    {
        return mBuildDB;
    }

    [[nodiscard]] const fs::path& GetInstallDir() const
    {
        return mInstallDir;
    }

private:
    std::tuple<Builders...> mBuilders;

    fs::path mProjectDir;
    fs::path mAssetDir;
    fs::path mInstallDir;

    std::flat_map<fs::path, IBuilder*> mBuilderDispatch;
    std::flat_map<fs::path, std::vector<BuildRecord>> mBuildDB;
};

sj::AssetDB BuildAssetDB(const std::filesystem::path& projectPath,
                         std::ranges::range auto buildRecords)
{
    sj::AssetDB assetDB;

    auto toAssetDbPath = [&](const std::filesystem::path& path) {
        return std::filesystem::relative(path, projectPath);
    };

    // Load already created imports, defer the rest to make sure we don't accidentally duplicate an
    // asset id
    std::vector<const BuildRecord*> pendingImports;
    for(const BuildRecord& item : buildRecords)
    {
        sj::ImportRecord record;

        if(fs::exists(item.importPath))
        {
            sj::ImportRecord record = sj::AssetDB::LoadImport(item.importPath.string());
            assetDB.AddImport(record.asset_id, toAssetDbPath(item.installPath).string());
        }
        else
        {
            pendingImports.emplace_back(&item);
        }
    }

    // Create new imports for stuff we're gonna build now that all previously used IDs are reserved
    {
        std::vector<char> writeBuffer;
        for(const BuildRecord* record : pendingImports)
        {
            sj::ImportRecord import {.asset_id = assetDB.NewAssetID()};
            assetDB.AddImport(import.asset_id, toAssetDbPath(record->installPath).string());

            glz::error_ctx error =
                glz::write_file_json(import, record->importPath.string(), writeBuffer);
            SJ_ASSERT(error.ec == glz::error_code::none,
                      "Failed to create import record at path {}",
                      record->importPath.string());
        }
    }

    return assetDB;
}

int main(int argc, const char* argv[])
{
    SJ_ENGINE_LOG_INFO("Starting data build.");
    CommandLineArgs args = ParseCommandLineArgs(std::span {argv, static_cast<size_t>(argc)});
    SJ_ENGINE_LOG_INFO("Arguments: {}", *glz::write<glz::opts {.format = glz::JSON}>(args));

    BuildContext<SceneBuilder, MeshBuilder, TextureBuilder> buildCtx(args.projectDir);

    // Build asset database
    sj::AssetDB assetDB = BuildAssetDB(args.projectDir, buildCtx.GetAllItems());

    // Build data
    for(const auto&& [extension, items] : buildCtx.GetBuildDB())
    {
        IBuilder* builder = buildCtx.GetBuilder(extension);
        for(const BuildRecord& item : items)
        {
            SJ_ENGINE_LOG_INFO("Building Item: {} -> {}",
                               item.assetPath.string(),
                               item.installPath.string());

            bool success = builder->BuildItem(item.assetPath, item.installPath);

            if(!success)
                SJ_ENGINE_LOG_ERROR("Failed to build item {}", item.assetPath.string());
        }
    }

    assetDB.Save((buildCtx.GetInstallDir() / ".AssetDB").string());
    assetDB.Save<glz::JSON>((buildCtx.GetInstallDir() / "human.AssetDB").string());

    std::print("Build Complete!");
}
