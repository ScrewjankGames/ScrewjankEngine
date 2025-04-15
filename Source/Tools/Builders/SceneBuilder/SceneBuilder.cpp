// Parent Include
#include "SceneBuilder.hpp"

// Builder Includes
#include <Utils.hpp>

// Shared Includes
#include <ScrewjankShared/io/File.hpp>
#include <ScrewjankShared/DataDefinitions/ScenePrototype.hpp>
#include <ScrewjankShared/DataDefinitions/TypeRegistration.hpp>
#include <ScrewjankShared/utils/Assert.hpp>
#include "ScrewjankShared/Math/Mat44.hpp"
#include "ScrewjankShared/string/StringHash.hpp"
#include "ScrewjankShared/utils/Log.hpp"

// Library Includes
#include <glaze/beve/read.hpp>
#include <glaze/beve/write.hpp>
#include <glaze/core/common.hpp>
#include <glaze/core/context.hpp>
#include <glaze/core/opts.hpp>
#include <glaze/core/read.hpp>
#include <glaze/core/reflect.hpp>
#include <glaze/json/json_t.hpp>
#include <glaze/json/read.hpp>
#include <glaze/glaze.hpp>

// STD Includes
#include <filesystem>
#include <cstdio>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

import sj.shared.containers;
import sj.shared.datadefs;

template <>
struct glz::meta<sj::ComponentSchema> {
   using T = sj::ComponentSchema;
   static constexpr auto value = object(
      "type", &T::type
   );
   
   static constexpr auto unknown_read{&T::unknown_read};
   // static constexpr auto unknown_write{&T::my_unknown_write};
};

struct GameObjectSchema
{
    sj::StringHash id;
    sj::dynamic_vector<sj::ComponentSchema> components;
};

struct SceneSchema
{
    sj::StringHash scene_name; 
    uint32_t memory; // Free ram allocated to this scene
    sj::dynamic_vector<GameObjectSchema> game_objects;
};

namespace glz
{
    template <>
    struct from<JSON, sj::StringHash>
    {
        template <auto Opts>
        static void op(sj::StringHash& strHash, auto&&... args)
        {
            std::string str;
            glz::parse<JSON>::op<Opts>(str, args...);

            strHash = sj::StringHash(str);
        }
    };

    template <>
    struct from<JSON, sj::Vec3>
    {
        template <auto Opts>
        static void op(sj::Vec3& vec, auto&&... args)
        {
            std::array<float, 3> data;
            glz::parse<JSON>::op<Opts>(data, args...);

            vec = {data[0], data[1], data[2]};
        }
    };

    template <>
    struct to<BEVE, sj::Vec3>
    {
        template <auto Opts>
        static void op(sj::Vec3& vec, auto&&... args)
        {
            std::array<float, 3> data = {vec.x, vec.y, vec.z};
            glz::serialize<BEVE>::op<Opts>(data, args...);
        }
    };

    template <>
    struct from<JSON, sj::Mat44>
    {
        template <auto Opts>
        static void op(sj::Mat44& transform, auto&&... args)
        {
            struct MatLayout
            {
                sj::Vec3 translation;
                sj::Vec3 rotation;
                sj::Vec3 scale;
            } layout;

            glz::parse<JSON>::op<Opts>(layout, args...);

            transform = sj::BuildTransform(layout.scale, layout.rotation, layout.translation);
        }
    };

    template <>
    struct to<BEVE, sj::Mat44>
    {
        template <auto Opts>
        static void op(sj::Mat44& m, auto&&... args)
        {
            std::array<float, 16> data = 
            {
                m[0][0], m[0][1], m[0][2], m[0][3],
                m[1][0], m[1][1], m[1][2], m[1][3],
                m[2][0], m[2][1], m[2][2], m[2][3],
                m[3][0], m[3][1], m[3][2], m[3][3]
            };

            glz::serialize<BEVE>::op<Opts>(data, args...);
        }
    };

    template <>
    struct from<BEVE, sj::Mat44>
    {
        template <auto Opts>
        static void op(sj::Mat44& m, auto&&... args)
        {
            std::array<float, 16> data;
            glz::parse<BEVE>::op<Opts>(data, args...);

            sj::Vec4 x (data[0], data[1], data[2], data[3]);
            sj::Vec4 y (data[4], data[5], data[6], data[7]);
            sj::Vec4 z (data[8], data[9], data[10], data[11]);
            sj::Vec4 w (data[12], data[13], data[14], data[15]);

            m = {x, y, z, w};
        }
    };
}

namespace sj::build
{
struct SceneHeader
{
    sj::StringHash scene_name; 
    uint32_t memory; // Free ram allocated to this scene
    uint32_t num_game_objects;
};

bool SceneBuilder::BuildItem(const std::filesystem::path& item, const std::filesystem::path& output_path) const
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

    SceneHeader outputHeader;
    outputHeader.scene_name = parsedScene.scene_name;
    outputHeader.memory = parsedScene.memory;

    std::filesystem::path scenePathNoExtension = output_path.parent_path() / output_path.stem();
    const char* tmp = scenePathNoExtension.c_str();

    for(const GameObjectSchema& go : parsedScene.game_objects)
    {
        outputHeader.num_game_objects++;
        std::filesystem::path gameObjectDir = 
            scenePathNoExtension 
            / (std::to_string(outputHeader.scene_name.AsInt()) + std::to_string(outputHeader.scene_name.AsInt()));
            
        std::filesystem::create_directories(gameObjectDir);

        // Group together components by type
        std::map<sj::TypeId, std::vector<ComponentSchema>> componentLists;

        for(const ComponentSchema& component : go.components)
        {
            sj::TypeId componentType = sj::StringHash(component.type).AsInt();

            std::vector<ComponentSchema>& entry = componentLists[componentType];
            entry.emplace_back(component);
        }

        // for this go, write out component lists
        for(const auto& entry : componentLists)
        {
            const sj::ComponentRegistration& registration = sj::TypeRegistry::GetComponentRegistration(entry.first);

            std::vector<std::byte> buffer;
            {
                const std::vector<ComponentSchema>& components = entry.second;
                registration.m_serializationFuncs.toBeveFn(components, buffer);
            }
            
            // prune 'sj::'
            std::string_view typeName = registration.m_typeName.substr(4);
            std::filesystem::path componentBufferPath = gameObjectDir / typeName;
            componentBufferPath.replace_extension(".beve");
            
            sj::File outputFile;
            outputFile.Open(componentBufferPath.c_str(), File::OpenMode::kWriteBinary);
            outputFile.Write(buffer.data(), buffer.size());
            outputFile.Close();

        }
    }

    return errorCtx.ec == glz::error_code::none;
}

}