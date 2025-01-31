#pragma once

#include <filesystem>
#include <string_view>
#include <vector>

namespace build_tool::utils
{
	inline void GlobFiles(const std::filesystem::path& root, std::span<const char* const> extensions, std::vector<std::filesystem::path>& out_files)
	{
		for(std::string_view extension : extensions)
		{
			for (const auto& entry : std::filesystem::directory_iterator(root)) 
			{
				if (entry.is_regular_file() && entry.path().extension() == extension) 
					out_files.emplace_back(entry.path());
				else if(entry.is_directory())
					GlobFiles(entry.path(), extensions, out_files);
			}
		}
	}
}