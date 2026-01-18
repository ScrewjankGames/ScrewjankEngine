module;
#include <ScrewjankStd/Log.hpp>

#include <glaze/glaze.hpp>

#include <string>
#include <string_view>
#include <filesystem>

export module sj.engine.core.Config;
export import sj.engine.config;

import sj.engine.system.threading;

import sj.datadefs.Serialization;

import sj.std.containers.map;
import sj.std.containers.vector;
import sj.std.math;
import sj.std.string_hash;

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
    std::string program_name;
    std::string default_scene;
    Vec2 window_size;
    InputBindings input_bindings;
};

Config LoadConfig()
{
    std::filesystem::path path = priv::FindConfig();

    Config config;
    
    scratchpad_scope scratchpad = ThreadContext::GetScratchpad();
    sj::dynamic_vector<char> buffer(&scratchpad.get_allocator());
    glz::error_ctx ctx = glz::read_file_json(config, path.c_str(), buffer);

    if(ctx.ec != glz::error_code::none)
    {
        std::string error = glz::format_error(ctx);
        SJ_ENGINE_LOG_FATAL("Failed to load configuration file @ {}. Resaon: {}",
                            path.c_str(),
                            error);
    }

    return config;
}
} // namespace sj