module;
#include "BuildTool_Config_generated.hpp"
#include "Utils.hpp"

#include <ScrewjankStd/Log.hpp>

#include <vector>
#include <span>
#include <filesystem>

export module sj.build.IGlobBuilder;

export namespace sj::build
{
    class IGlobBuilder
    {
    public:
        virtual ~IGlobBuilder() = default;

        [[nodiscard]] virtual std::span<const char* const> GetExtensions() const = 0;
        [[nodiscard]] virtual const char* GetBuilderName() const = 0;
        [[nodiscard]] virtual const char* GetOutputExtension() const = 0;

        [[nodiscard]] virtual bool BuildItem(const std::filesystem::path& item,
                                             const std::filesystem::path& output_path) = 0;

        void BuildAll()
        {
            std::vector<std::filesystem::path> engine_items;
            utils::GlobFiles(config::EngineAssetDir, GetExtensions(), engine_items);
            BuildList(engine_items, config::EngineAssetDir, config::EngineDataDir);

            std::vector<std::filesystem::path> game_items;
            utils::GlobFiles(config::GameAssetDir, GetExtensions(), game_items);
            BuildList(game_items, config::GameAssetDir, config::GameDataDir);
        }

    private:
        void BuildList(std::span<const std::filesystem::path> items,
                       const char* input_dir,
                       const char* output_dir)
        {
            for(const auto& path : items)
            {
                SJ_ENGINE_LOG_INFO("Building item {}", path.c_str());
                const std::filesystem::path outputItemPath =
                    utils::GetDestinationPath(path,
                                              input_dir,
                                              output_dir,
                                              GetOutputExtension());

                std::filesystem::create_directories(outputItemPath.parent_path());

                bool success = BuildItem(path, outputItemPath);
                if(!success)
                {
                    SJ_ENGINE_LOG_ERROR("Failed to build item {}. Build incomplete",
                                        path.filename().c_str());
                }
            }
        }
    };
} // namespace sj::build