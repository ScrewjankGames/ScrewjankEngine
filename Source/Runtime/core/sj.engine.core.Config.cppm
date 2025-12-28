module;
#include <ScrewjankStd/Log.hpp>

#include <glaze/glaze.hpp>

#include <string>
#include <string_view>
#include <filesystem>

export module sj.engine.core.Config;
export import sj.engine.config;

import sj.datadefs.Serialization;

import sj.std.string_hash;
import sj.std.containers.map;
import sj.std.containers.vector;

namespace priv
{
std::filesystem::path FindConfig()
{
    for(auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path()))
    {
        if(entry.is_directory())
            continue;

        std::filesystem::path path = entry.path();
        const auto& extension = entry.path().extension();
        [[maybe_unused]] auto dummy = extension.string();
        if(extension == ".sj_proj")
            return path;
    }

    return {};
}
} // namespace priv

export namespace sj
{

struct Config
{
    std::string_view default_scene;
    InputBindings input_bindings;
};

struct ConfigHandle
{
    Config config;
    std::vector<char> raw_data;
};

ConfigHandle LoadConfig()
{
    std::filesystem::path path = priv::FindConfig();

    ConfigHandle handle;
    glz::error_ctx ctx = glz::read_file_json(handle.config, path.c_str(), handle.raw_data);

    if(ctx.ec != glz::error_code::none)
    {
        std::string error = glz::format_error(ctx);
        SJ_ENGINE_LOG_FATAL("Failed to load configuration file @ {}. Resaon: {}",
                            path.c_str(),
                            error);
    }

    return handle;
}
} // namespace sj