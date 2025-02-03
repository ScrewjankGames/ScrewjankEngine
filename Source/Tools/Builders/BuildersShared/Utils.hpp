#pragma once

#include <filesystem>
#include <string_view>
#include <vector>

namespace sj::build::utils
{
	inline void GlobFiles(const std::filesystem::path& root, std::span<const char* const> extensions, std::vector<std::filesystem::path>& out_files)
	{
		auto test = root.c_str();
		for(std::string_view extension : extensions)
		{
			for (const auto& entry : std::filesystem::directory_iterator(root)) 
			{
				test = entry.path().c_str();
				auto fileExtension = entry.path().extension();
				if (entry.is_regular_file() && entry.path().extension() == extension) 
					out_files.emplace_back(entry.path());
				else if(entry.is_directory())
					GlobFiles(entry.path(), extensions, out_files);
			}
		}
	}

	inline std::filesystem::path GetDestinationPath(
		std::filesystem::path item, 
		std::filesystem::path input_dir, 
		std::filesystem::path output_dir,
		std::string_view output_extension)
	{
		std::filesystem::path relativePath = std::filesystem::relative(item, input_dir);
		std::filesystem::path outputPath = output_dir / relativePath;
		outputPath = outputPath.replace_extension(output_extension);

	    return outputPath;
	}
}