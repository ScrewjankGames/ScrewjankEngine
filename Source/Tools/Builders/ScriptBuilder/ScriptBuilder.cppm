module;

#include <Luau/BytecodeBuilder.h>
#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/Log.hpp>

#include <Luau/Compiler.h>

#include <filesystem>
#include <span>
#include <string_view>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

export module sj.build.ScriptBuilder;
import sj.std;
import sj.builders;
import sj.datadefs;

namespace stdfs = std::filesystem;

export namespace sj::build
{
class ScriptBuilder final : public IBuilder
{
public:
    [[nodiscard]] std::span<const std::string_view> GetExtensions() const override
    {
        using namespace std::literals;
        static constexpr std::array<std::string_view, 1> extensions = {".luau"sv};
        return extensions;
    }

    [[nodiscard]] std::string_view GetBuilderName() const override
    {
        return "GLB Builder";
    }

    bool BuildItem(BuildContext& in_ctx, const stdfs::path& item) override
    {
        auto&& [output_path, id] = in_ctx.Import(item, item.filename().replace_extension(".luauc"));

        std::ifstream fileStream(item);
        if(!fileStream.is_open())
            return false;

        const std::string source = [&] -> std::string {
            std::stringstream stringStream;
            stringStream << fileStream.rdbuf(); // Read the file buffer into the stream
            return stringStream.str();
        }();

        Luau::BytecodeBuilder builder(nullptr);
        Luau::CompileOptions options = {};
        try
        {
            Luau::compileOrThrow(builder, source, options);
        }
        catch(Luau::CompileError& e)
        {
            SJ_ENGINE_LOG_ERROR("Script Builder Compilation Failure: {}", e.what());
            return false;
        }


        std::ofstream outputStream(output_path);
        outputStream << builder.getBytecode();

        return true;
    }
};
} // namespace sj::build