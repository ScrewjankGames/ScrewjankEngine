#pragma once
#include <BuilderTypes.hpp>

namespace sj::build
{
	class SceneBuilder final : public GlobBuilder
	{
	public:
		std::span<const char* const> GetExtensions() override 
		{ 
			static constexpr std::array extensions = {".scene" };
			return extensions;
		}

		const char* GetBuilderName() override { return "Scene Builder"; }
		const char* GetOutputExtension() override { return ".sj_scene"; }
        
        bool BuildItem(const std::filesystem::path& item, const std::filesystem::path& output_path) const override;
	};
}
