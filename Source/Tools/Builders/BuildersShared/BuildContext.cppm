module;
#include <ScrewjankStd/Log.hpp>

#include <glaze/glaze.hpp>

#include <flat_map>
#include <filesystem>
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <string_view>
#include <unordered_map>

export module sj.builders:BuildContext;
import :IBuilder;
import :Utils;

import sj.std;
import sj.datadefs;

namespace stdfs = std::filesystem;

export namespace sj::build
{
class BuildContext
{
public:
    template <class... Builders>
    static BuildContext
    Create(std::string_view projectDir, std::string_view assetDir, std::string_view installDir)
    {
        std::vector<std::unique_ptr<IBuilder>> builders;
        (builders.emplace_back(new Builders()), ...);

        return BuildContext(projectDir, assetDir, installDir, std::move(builders));
    }

    void Scan(std::string_view assetDir)
    {
        for(const stdfs::directory_entry& entry : stdfs::recursive_directory_iterator(assetDir))
        {
            if(!entry.is_regular_file())
                continue;

            stdfs::path itemPath = entry.path();

            if(!mBuilderDispatch.contains(itemPath.extension()))
                continue;

            BuildRecord& record = mBuildDb[itemPath];
            record.importPath = utils::GetImportPath(itemPath);
            const bool importExists = stdfs::exists(record.importPath);

            if(importExists)
            {
                record.importRecord = ImportRecord::Load(record.importPath.string());

                for(auto&& [file_name, id] : record.importRecord.installed_assets)
                    mAssetDb.AddImport(id, ComputeInstallFilename(file_name, id).string());
            }
        }
    }

    void Build()
    {
        std::vector<char> importWriteBuffer;
        // Build data
        for(const auto&& [itemPath, record] : mBuildDb)
        {
            IBuilder* builder = mBuilderDispatch[itemPath.extension()];
            SJ_ENGINE_LOG_INFO("Building Item: {}", itemPath.string());

            bool success = builder->BuildItem(*this, itemPath);

            if(!success)
            {
                SJ_ENGINE_LOG_ERROR("Failed to build item {}", itemPath.string());
                continue;
            }

            glz::error_ctx err = glz::write_file_json(record.importRecord,
                                                      record.importPath.string(),
                                                      importWriteBuffer);
            if(err != glz::error_code::none)
                SJ_ENGINE_LOG_ERROR("Failed to write import {} for item {}",
                                    record.importPath.string(),
                                    itemPath.string());
        }

        mAssetDb.Save((mAbsInstallDir / ".AssetDB").string());
        mAssetDb.Save<glz::JSON>((mAbsInstallDir / "human.AssetDB").string());
    }

    std::pair<stdfs::path, AssetID> Import(const stdfs::path& parentFilePath, stdfs::path fileName)
    {
        SJ_ENGINE_LOG_INFO("Importing asset: {}", fileName.string());

        ImportRecord& record = mBuildDb[parentFilePath].importRecord;

        auto installIt = record.installed_assets.find(fileName);
        if(installIt == record.installed_assets.end())
            installIt = record.installed_assets.emplace(fileName, mAssetDb.NewAssetID()).first;

        AssetID id = installIt->second;
        stdfs::path installFileName = ComputeInstallFilename(fileName, id);

        mAssetDb.AddImport(id, (mRelInstallDir / installFileName).string());
        return std::pair {mAbsInstallDir / installFileName, id};
    }

    IBuilder* GetBuilderByExtension(std::string_view ext)
    {
        auto it = mBuilderDispatch.find(ext);
        if(it == mBuilderDispatch.end())
            return nullptr;

        return it->second;
    }

private:
    struct BuildRecord
    {
        stdfs::path importPath;
        ImportRecord importRecord;
    };

    static std::filesystem::path ComputeInstallFilename(const std::filesystem::path& fileName,
                                                        AssetID id)
    {
        std::string idPart = std::format("{:#16X}_", id);
        return std::filesystem::path(idPart + fileName.string());
    }

    BuildContext(std::string_view projectDir,
                 std::string_view assetDir,
                 std::string_view installDir,
                 std::vector<std::unique_ptr<IBuilder>>&& builders)
        : mAbsProjectDir(projectDir), mAbsInstallDir(installDir), mBuilders(std::move(builders))
    {
        mRelInstallDir = std::filesystem::relative(mAbsInstallDir, mAbsProjectDir);

        for(const std::unique_ptr<IBuilder>& builder : mBuilders)
            for(std::string_view extension : builder->GetExtensions())
                mBuilderDispatch[extension] = builder.get();
    }

    AssetDB mAssetDb;

    std::filesystem::path mAbsProjectDir;
    std::filesystem::path mAbsInstallDir;
    std::filesystem::path mRelInstallDir;

    std::vector<std::unique_ptr<IBuilder>> mBuilders;
    std::flat_map<stdfs::path, IBuilder*> mBuilderDispatch;
    std::flat_map<stdfs::path, BuildRecord> mBuildDb;
};
} // namespace sj::build