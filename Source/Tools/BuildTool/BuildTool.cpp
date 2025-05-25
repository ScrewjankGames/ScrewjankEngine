#include "BuildTool_Config_generated.hpp"

#include "TextureBuilder.hpp"
#include "ModelBuilder.hpp"
#include "SceneBuilder.hpp"

#include <cstdlib>
#include <print>
#include <filesystem>

int main()
{
	for (const char* path : sj::build::config::RequiredDirs) 
	{
		std::filesystem::create_directories(path);
	}

	sj::build::TextureBuilder texture_builder;
	texture_builder.BuildAll();

	sj::build::ModelBuilder model_builder;
	model_builder.BuildAll();

	sj::build::SceneBuilder scene_builder;
	scene_builder.BuildAll();

	std::print("Build Complete. Press any key to close...");
}
