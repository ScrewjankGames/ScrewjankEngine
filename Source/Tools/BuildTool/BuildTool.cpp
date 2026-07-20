#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/Log.hpp>

#include <glaze/glaze.hpp>

#include <print>
#include <filesystem>
#include <functional>
#include <flat_map>
#include <string_view>
#include <vector>
#include <ranges>
#include <tuple>

import sj.std;
import sj.builders;
import sj.datadefs;

import sj.TextureBuilder;
import sj.SceneBuilder;
import sj.MeshBuilder;
import sj.build.GlbBuilder;
import sj.build.ScriptBuilder;


using namespace sj::build;

struct CommandLineArgs
{
    std::string_view projectDir;
    std::string_view assetDir;
    std::string_view installDir;
};

CommandLineArgs ParseCommandLineArgs(std::span<const char*> commandLine)
{
    CommandLineArgs args;

    auto cursor = commandLine.begin() + 1;
    auto tryConsumeFn = [&](std::string_view arg, std::string_view& out) -> bool {
        if(std::string_view(*cursor) == arg)
        {
            ++cursor;
            SJ_ASSERT(cursor != commandLine.end(),
                      "Failed to parse command line arg {}. Missing argument",
                      arg);

            out = *cursor;
            return true;
        }

        return false;
    };

    while(cursor != commandLine.end())
    {
        if(tryConsumeFn("--project-dir", args.projectDir))
            continue;

        if(tryConsumeFn("--asset-dir", args.assetDir))
            continue;

        if(tryConsumeFn("--data-install-dir", args.installDir))
            continue;

        ++cursor;
    }

    return args;
}

int main(int argc, const char* argv[])
{
    SJ_ENGINE_LOG_INFO("Starting data build.");
    CommandLineArgs args = ParseCommandLineArgs(std::span {argv, static_cast<size_t>(argc)});
    SJ_ENGINE_LOG_INFO("Arguments: {}", *glz::write<glz::opts {.format = glz::JSON}>(args));

    BuildContext buildCtx =
        BuildContext::Create<SceneBuilder, MeshBuilder, TextureBuilder, GlbBuilder, ScriptBuilder>(
            args.projectDir,
            args.assetDir,
            args.installDir);

    buildCtx.Scan(args.assetDir);
    buildCtx.Build();

    std::print("Build Complete!");
}
