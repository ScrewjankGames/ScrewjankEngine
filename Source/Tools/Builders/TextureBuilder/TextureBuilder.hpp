#pragma once
#include <BuilderTypes.hpp>

namespace sj::build
{
	class TextureBuilder final : public GlobBuilder
	{
	public:
		std::span<const char* const> GetExtensions() override 
		{ 
			static constexpr std::array extensions = {".png", ".jpg"};
			return extensions;
		}

		const char* GetBuilderName() override { return "Texture Builder"; }
		const char* GetOutputExtension() override { return ".sj_tex"; }
        
        bool BuildItem(const std::filesystem::path& item, const std::filesystem::path& output_path) const override;
	};
}
