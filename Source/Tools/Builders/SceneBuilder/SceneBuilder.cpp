// Parent Include
#include "SceneBuilder.hpp"

// Builder Includes
#include <Utils.hpp>

// SJSTD Includes
#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/TypeMacros.hpp>
#include <ScrewjankStd/Log.hpp>

// Library Includes
#include <glaze/beve/read.hpp>
#include <glaze/beve/write.hpp>
#include <glaze/core/common.hpp>
#include <glaze/core/context.hpp>
#include <glaze/core/opts.hpp>
#include <glaze/core/read.hpp>
#include <glaze/core/reflect.hpp>
#include <glaze/core/meta.hpp>
#include <glaze/json/json_t.hpp>
#include <glaze/json/read.hpp>
#include <glaze/glaze.hpp>

// STD Includes
#include <filesystem>
#include <string>
#include <vector>

import sj.std.containers;
import sj.datadefs;
import sj.std.math;

struct ComponentSchema
{
    std::string type = {};
    glz::json_t componentJson = glz::json_t::object_t{};

    void unknown_read(const glz::sv& key, const glz::raw_json& value) 
    {
        glz::json_t::object_t& obj = componentJson.get_object();

        glz::json_t asJson;
        glz::error_ctx res = glz::read_json(asJson, value.str);
        SJ_ASSERT(res.ec == glz::error_code::none, "failed to parse raw json");

        obj[std::string(key)] = asJson;
    }
};

template <>
struct glz::meta<ComponentSchema> {
   using T = ComponentSchema;
   static constexpr auto value = object(
      "type", &T::type
   );
   
   static constexpr auto unknown_read{&T::unknown_read};
   // static constexpr auto unknown_write{&T::my_unknown_write};
};

struct GameObjectSchema
{
    sj::string_hash id;
    std::vector<ComponentSchema> components;
};

struct SceneSchema
{
    sj::string_hash scene_name; 
    uint32_t memory = 0; // Free ram allocated to this scene
    std::vector<GameObjectSchema> game_objects;
};

namespace sj::build
{

bool SceneBuilder::BuildItem(const std::filesystem::path& item, const std::filesystem::path& output_path)
{
    SceneSchema parsedScene;
    std::string buffer;
    glz::error_ctx errorCtx = glz::read_file_json<glz::opts{.error_on_unknown_keys=false}>(parsedScene, item.c_str(), buffer);
    
    if(errorCtx.ec != glz::error_code::none)
    {
        std::string descriptive_error = glz::format_error(errorCtx, buffer);
        SJ_ENGINE_LOG_ERROR("Failed to parse scene {}. Error: {}", item.c_str(), descriptive_error);
        return false;
    }

    SceneChunk outputScene;
    outputScene.name = parsedScene.scene_name;
    outputScene.memory = parsedScene.memory;
    outputScene.gameObjects.reserve(parsedScene.game_objects.size());

    for(const GameObjectSchema& go : parsedScene.game_objects)
    {
        GameObjectChunk goChunk;
        goChunk.id = go.id;
        goChunk.components.reserve(go.components.size());

        for(const ComponentSchema& component : go.components)
        {
            DataChunk componentChunk;
            string_hash typeHash(component.type);
            TypeId typeId = typeHash.AsInt();
            componentChunk.type = typeId;

            WriteChunkFn writeFn = g_chunkWriterFuncs.get(typeId);
            writeFn(component.componentJson, componentChunk);

            goChunk.components.emplace_back(std::move(componentChunk));
        }

        outputScene.gameObjects.emplace_back(std::move(goChunk));
    }

    errorCtx = glz::write_file_beve(outputScene, output_path.c_str(), std::string{});
    
    if(errorCtx.ec != glz::error_code::none)
    {
        std::string descriptive_error = glz::format_error(errorCtx, buffer);
        SJ_ENGINE_LOG_ERROR("Failed to parse scene {}. Error: {}", item.c_str(), descriptive_error);
        return false;
    }
    else
    {
        return true;
    }
}

}
