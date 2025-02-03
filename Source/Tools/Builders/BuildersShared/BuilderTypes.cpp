#include "BuilderTypes.hpp"
#include "BuildTool_Config_generated.hpp"
#include "Utils.hpp"

#include <filesystem>
#include <print>
#include <vector>

namespace sj::build
{
    void GlobBuilder::BuildAll()
    {
        std::vector<std::filesystem::path> engine_items;
        utils::GlobFiles(config::EngineAssetDir, GetExtensions(), engine_items);

        std::vector<std::filesystem::path> game_items;
        utils::GlobFiles(config::GameAssetDir, GetExtensions(), game_items);

        for (const auto& path : engine_items)
        {
            std::println("Building item {}", path.c_str());
            const std::filesystem::path outputItemPath = 
                utils::GetDestinationPath(
                    path, 
                    config::EngineAssetDir, 
                    config::EngineDataDir, 
                    GetOutputExtension()
                );

            std::filesystem::create_directories(outputItemPath.parent_path());

            bool success = BuildItem(path, outputItemPath);
            if (!success)
            {
                std::println("Failed to build item {}. Build aborted", path.filename().c_str());
                return;
            }
        }

        for (const auto& path : game_items)
        {
            std::println("Building item {}", path.c_str());
            
            const std::filesystem::path outputItemPath = 
                utils::GetDestinationPath(
                    path, 
                    config::GameAssetDir, 
                    config::GameDataDir, 
                    GetOutputExtension()
                );

            std::filesystem::create_directories(outputItemPath);

            bool success = BuildItem(path, outputItemPath);
            if (!success)
            {
                std::println("Failed to build item {}. Build aborted", path.filename().c_str());
                return;
            }
        }
    }
}
