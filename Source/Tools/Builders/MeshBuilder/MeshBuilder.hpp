#pragma once
#include <span>
#include <filesystem>
import sj.build.IGlobBuilder;

namespace sj::build
{
	class MeshBuilder final : public IGlobBuilder
	{
	public:
		[[nodiscard]] std::span<const char* const> GetExtensions() const override
		{ 
			static constexpr std::array extensions = { ".obj" };
			return extensions;
		}

		[[nodiscard]] const char* GetBuilderName() const override { return "Mesh Builder"; }
		[[nodiscard]] const char* GetOutputExtension() const override { return ".sj_mesh"; }
        
        bool BuildItem(const std::filesystem::path& item, const std::filesystem::path& output_path) override;
	};
}
