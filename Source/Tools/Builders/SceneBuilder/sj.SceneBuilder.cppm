module;

#include <ScrewjankStd/Assert.hpp>

#include <glaze/json/read.hpp>
#include <glaze/beve/write.hpp>
#include <glaze/core/context.hpp>

#include <span>
#include <filesystem>

export module sj.SceneBuilder;
import sj.builders;
import sj.datadefs;
import sj.std.primitives;
import sj.std.type_info;

export namespace sj::build
{
class SceneBuilder final : public IBuilder
{
public:
    [[nodiscard]] std::span<const std::string_view> GetExtensions() const override
    {
        using namespace std::literals;
        static constexpr std::array extensions = {".scene"sv};
        return extensions;
    }

    [[nodiscard]] std::string_view GetBuilderName() const override
    {
        return "Scene Builder";
    }

    bool BuildItem(BuildContext& in_ctx, const std::filesystem::path& item) override
    {
        auto&& [output_path, id] =
            in_ctx.Import(item, item.filename().replace_extension(".sj_scene"));

        std::vector<u8> readBuffer;

        SceneChunk sceneDef;

        glz::error_ctx errorCtx =
            glz::read_file_json<glz::opts {.error_on_unknown_keys = false}>(sceneDef,
                                                                            item.string().c_str(),
                                                                            readBuffer);

        if(errorCtx.ec != glz::error_code::none)
            return false;

        ChunkWriter chunkWriter;
        SceneChunkHeader sceneHeader {
            .scene_name = sceneDef.scene_name,
            .num_game_objects = static_cast<u32>(sceneDef.game_objects.size()),
        };
        chunkWriter.Write(sceneHeader);

        for(const GameObjectChunk& goDef : sceneDef.game_objects)
        {
            WriteGameObject(goDef, chunkWriter);
        }

        auto&& [output_path2, id2] =
            in_ctx.Import(item, item.filename().replace_extension(".sj_scene2"));

        std::ofstream file(output_path2, std::ios::binary);
        file.write(reinterpret_cast<const char*>(chunkWriter.GetBufferView().data()),
                   static_cast<std::streamsize>(chunkWriter.GetBufferView().size()));

        bool success =
            std::filesystem::copy_file(item,
                                       output_path,
                                       std::filesystem::copy_options::overwrite_existing);
        return success;
    }

private:
    void WriteGameObject(const GameObjectChunk& goDef, ChunkWriter& chunkWriter)
    {
        GameObjectChunkHeader goChunk {
            .name = goDef.id,
            .num_components = static_cast<u32>(goDef.components.size()),
            .num_children = static_cast<u32>(goDef.children.size()),

        };
        chunkWriter.Write(goChunk);

        std::vector<TypeId> componentTypes;

        for(const ComponentChunk& componentDef : goDef.components)
            componentTypes.emplace_back(componentDef.first.get_hash().AsInt());

        ComponentChunkHeader componentsHeader {
            .load_chunk_type_ids = componentTypes,
        };
        chunkWriter.Write(componentsHeader);

        for(const ComponentChunk& componentDef : goDef.components)
            chunkWriter.Write(componentDef.second);

        for(const GameObjectChunk& child : goDef.children)
            WriteGameObject(child, chunkWriter);
    }
};
} // namespace sj::build
