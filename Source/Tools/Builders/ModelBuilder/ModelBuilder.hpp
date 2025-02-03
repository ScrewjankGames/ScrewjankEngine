#pragma once
#include <BuilderTypes.hpp>

namespace sj::build
{
	class ModelBuilder final : public GlobBuilder
	{
	public:
		std::span<const char* const> GetExtensions() override 
		{ 
			static constexpr std::array extensions = { ".obj" };
			return extensions;
		}

		const char* GetBuilderName() override { return "Model Builder"; }
		const char* GetOutputExtension() override { return ".sj_mesh"; }
        
        bool BuildItem(const std::filesystem::path& item, const std::filesystem::path& output_path) const override;
	};
}
