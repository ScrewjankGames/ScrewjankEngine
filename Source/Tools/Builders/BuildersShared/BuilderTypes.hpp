#pragma once

#include <cstdlib>
#include <print>
#include <span>
#include <filesystem>

namespace sj::build
{
class GlobBuilder
{
public:
    virtual std::span<const char* const> GetExtensions() = 0;
    virtual const char* GetBuilderName() = 0;
    virtual const char* GetOutputExtension() = 0;

    virtual bool BuildItem(const std::filesystem::path& item, const std::filesystem::path& output_path) const = 0;
    
    void BuildAll();
};
}
