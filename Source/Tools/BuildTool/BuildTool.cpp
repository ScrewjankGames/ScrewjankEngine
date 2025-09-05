#include "BuildTool_Config_generated.hpp"

#include "MeshBuilder.hpp"
#include "SceneBuilder.hpp"

#include <cstdlib>
#include <print>
#include <filesystem>

import sj.TextureBuilder;

int main()
{
	for (const char* path : sj::build::config::RequiredDirs) 
	{
		std::filesystem::create_directories(path);
	}

	sj::build::TextureBuilder texture_builder;
	texture_builder.BuildAll();

	sj::build::MeshBuilder mesh_builder;
	mesh_builder.BuildAll();

	sj::build::SceneBuilder scene_builder;
	scene_builder.BuildAll();

	std::print("Build Complete. Press any key to close...");
}
