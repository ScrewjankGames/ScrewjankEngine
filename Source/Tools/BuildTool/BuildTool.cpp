#include "BuildTool_Config_generated.hpp"
#include "Utils.hpp"

#include <cstdlib>
#include <print>
#include <span>
#include <filesystem>

namespace build_tool 
{
	class GlobBuilder
	{
	public:
		virtual std::span<const char* const> GetExtensions() = 0;
		virtual const char* GetBuilderName() = 0;
		virtual const char* GetBuilderExePath() = 0;
		virtual const char* GetOutputExtension() = 0;

		bool BuildItem(std::filesystem::path item, std::filesystem::path input_dir, std::filesystem::path output_dir)
		{
			std::print("{0} building {1}", GetBuilderName(), item.filename().c_str());
			
			std::filesystem::path newPath = std::format("{0}{1}", output_dir.c_str(), std::filesystem::relative(item, input_dir).c_str());
			newPath = newPath.replace_extension(GetOutputExtension());
			std::filesystem::create_directories(newPath);

			std::string args = std::format(" {0} {1}", item.c_str(), newPath.c_str());

			std::filesystem::path builderExeDir = config::BuildersBinDir;
			std::filesystem::path builderExePath = builderExeDir.append(GetBuilderExePath());

			std::string command = builderExePath.string() + args;
			int res = std::system(command.c_str());

			if (res != 0)
			{
				std::print("{0} {1} Failed!", GetBuilderName(), item.filename().c_str());
				return false;
			}

			return true;
		}

		void BuildAll()
		{
			std::vector<std::filesystem::path> engine_items;
			utils::GlobFiles(config::EngineAssetDir, GetExtensions(), engine_items);

			std::vector<std::filesystem::path> game_items;
			utils::GlobFiles(config::GameAssetDir, GetExtensions(), game_items);

			for (const auto& path : engine_items)
			{
				bool success = BuildItem(path, config::EngineAssetDir, config::EngineDataDir);
				if (!success)
				{
					std::print("Failed to build item {}. Build aborted", path.filename().c_str());
					return;
				}
			}

			for (const auto& path : game_items)
			{
				bool success = BuildItem(path, config::GameAssetDir, config::GameDataDir);
				if (!success)
				{
					std::print("Failed to build item {}. Build aborted", path.filename().c_str());
					return;
				}
			}
		}
	};

	class TextureBuilder : public GlobBuilder
	{
	public:
		std::span<const char* const> GetExtensions() override 
		{ 
			static constexpr std::array extensions = {"*.png", "*.jpg"};
			return extensions;
		}

		const char* GetBuilderName() override { return "Texture Builder"; }
		const char* GetBuilderExePath() override { return "TextureBuilder/Release/TextureBuilder.exe"; }
		const char* GetOutputExtension() override { return ".sj_tex"; }
	};

	class ModelBuilder : public GlobBuilder
	{
	public:
		std::span<const char* const> GetExtensions() override 
		{ 
			static constexpr std::array extensions = {"*.obj" };
			return extensions;
		}

		const char* GetBuilderName() override { return "Model Builder"; }
		const char* GetBuilderExePath() override { return "ModelBuilder/Release/ModelBuilder.exe"; }
		const char* GetOutputExtension() override { return ".sj_mesh"; }
	};

	class SceneBuilder : public GlobBuilder 
	{
	public:
		std::span<const char* const> GetExtensions() override 
		{ 
			static constexpr std::array extensions = {"*.scene" };
			return extensions;
		}

		const char* GetBuilderName() override { return "Scene Builder"; }
		const char* GetBuilderExePath() override { return "ceneBuilder/Release/SceneBuilder.exe"; }
		const char* GetOutputExtension() override { return ".sj_scene"; }
	};
}

int main(int argc, const char** argv)
{
	for (const char* path : build_tool::config::RequiredDirs) 
	{
		std::filesystem::create_directories(path);
	}

	build_tool::TextureBuilder texture_builder;
	texture_builder.BuildAll();

	build_tool::ModelBuilder model_builder;
	model_builder.BuildAll();

	build_tool::SceneBuilder scene_builder;
	scene_builder.BuildAll();

	std::print("Build Complete. Press any key to close...");
}
