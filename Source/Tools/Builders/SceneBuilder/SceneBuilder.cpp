// Parent Include
#include "SceneBuilder.hpp"

// Builder Includes
#include <Utils.hpp>

// Shared Includes
#include <ScrewjankShared/io/File.hpp>
#include <ScrewjankShared/DataDefinitions/ScenePrototype.hpp>
#include <ScrewjankShared/DataDefinitions/GameObjectPrototype.hpp>
#include <ScrewjankShared/DataDefinitions/Components/CameraComponent.hpp>
#include <ScrewjankShared/DataDefinitions/Components/ScriptComponent.hpp>
#include <ScrewjankShared/utils/Assert.hpp>
#include <ScrewjankShared/DataDefinitions/TypeRegistration.hpp>

// Library Includes
#include <glaze/core/common.hpp>
#include <glaze/core/context.hpp>
#include <glaze/core/opts.hpp>
#include <glaze/core/read.hpp>
#include <glaze/core/reflect.hpp>
#include <glaze/json/json_t.hpp>
#include <glaze/json/read.hpp>
#include <nlohmann/json.hpp>
#include <glaze/glaze.hpp>


// STD Includes
#include <cstdio>
#include <ranges>
#include <vector>
#include <fstream>
#include "ScrewjankShared/Math/Mat44.hpp"
#include "ScrewjankShared/string/StringHash.hpp"

struct ComponentSchema
{
    std::string type;
    std::map<glz::sv, glz::raw_json> componentData; // store key/raw_json in extra map
};

template <>
struct glz::meta<ComponentSchema> {
   using T = ComponentSchema;
   static constexpr auto value = object(
      "type", &T::type
   );
   
   static constexpr auto unknown_write{&T::componentData};
   static constexpr auto unknown_read{&T::componentData};
};

struct GameObjectSchema
{
    std::string id;
    std::vector<ComponentSchema> components;

};

struct SceneSchema
{
    sj::StringHash scene_name; 
    uint32_t memory; // Free ram allocated to this scene
    std::vector<GameObjectSchema> game_objects;
};

void ParseComponentJson(const glz::json_t::object_t& object, sj::ComponentData& out_data)
{
    const std::string& typeString = object.at("type").get_string();

    sj::TypeId typeId = sj::StringHash(typeString).AsInt();
    
    auto registry = sj::GetComponentRegistry();
    auto it = std::ranges::find(registry, typeId, &sj::ComponentRegistration::m_componentTypeId);

    SJ_ASSERT(it != registry.end(), "Failed to find runtime registration data of component type {}", typeString);

    const sj::ComponentRegistration& registration = *it;
    
    registration.m_serializationFuncs.fromJsonFn(object, out_data);
}

namespace glz
{
    // template <>
    // struct from<JSON, ComponentSchema>
    // {
    //     template <auto Opts>
    //     static void op(ComponentSchema& componentSchema, auto&&... args)
    //     {
    //         std::string type;
    //         glz::parse<JSON>::op<Opts>(type, args...);

    //         int a = 0;
    //         // glz::json_t componentJson;
    //         // glz::from<JSON, glz::json_t>::template op<Opts>(componentJson, args...);


    //         // ParseComponentJson(componentJson.get_object(), componentSchema.component);
    //     }
    // };
    template <>
    struct from<JSON, sj::GameObjectId>
    {
        template <auto Opts>
        static void op(sj::GameObjectId& godId, auto&&... args)
        {
            godId = {};
        }
    };

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
            glz::from<JSON, decltype(data)>::template op<Opts>(data, args...);

            vec = {data[0], data[1], data[2]};
        }
    };

    template <>
    struct from<JSON, sj::Mat44>
    {
        template <auto Opts>
        static void op(sj::Mat44& transform, auto&&... args)
        {
            sj::Vec3 translation;
            glz::from<JSON, sj::Vec3>::template op<Opts>(translation, args...);

            sj::Vec3 rotation;
            glz::from<JSON, sj::Vec3>::template op<Opts>(rotation, args...);

            sj::Vec3 scale;
            glz::from<JSON, sj::Vec3>::template op<Opts>(scale, args...);

            transform = sj::BuildTransform(scale, rotation, translation);
        }
    };
}

namespace sj::build
{

class typed_vector
{
public:
    typed_vector() = default;

    typed_vector(TypeId type, void* data, size_t count)
     : m_typeId(type), m_data(data), m_count(count), m_capacity(count)
    {

    }

    void SetTypeId(sj::TypeId id) { m_typeId = id; }

    template<class T> requires T::kTypeId
    void push_back(const T& element)
    {
        SJ_ASSERT(m_typeId == T::kTypeId, "Mismatched type ID");

        T* typedData = reinterpret_cast<T*>(m_data);


        if(m_count >= m_capacity)
        {
            reserve<T>(m_capacity * 2);
        }
        
        new (typedData[m_count]) T(element);
    }

    template<class T>
    void reserve(size_t newCount)
    {

    }

private:
    TypeId m_typeId;
    void* m_data;

    size_t m_count;
    size_t m_capacity;
};

struct ComponentBuffer
{
    TypeId typeId;
    std::vector<uint8_t> components_array;
};

struct ScenePrototype2
{
    StringHash name; 
    uint32_t memory; // Free ram allocated to this scene
    uint32_t scriptPoolSize; // Number of elements needed in the script pool
    std::vector<GameObject> gameObjects;
    std::vector<sj::ComponentBuffer> componentBuffers;
};

ComponentData BuildComponent(const ComponentSchema& data)
{
    glz::json_t::object_t component;
    for(const auto& entry : data.componentData)
    {
        glz::json_t json;
        glz::error_ctx err = glz::read<glz::opts{}>(json, entry.second.str); 
        SJ_ASSERT(err.ec == glz::error_code::none, "Failed to parse unkown keys");

        component[std::string(entry.first)] = json;
    }

    sj::ComponentData finalComponent;
    ParseComponentJson(component, finalComponent);
    return finalComponent;
}

bool SceneBuilder::BuildItem(const std::filesystem::path& item, const std::filesystem::path& output_path) const
{
    SceneSchema test;
    std::string buffer;
    glz::error_ctx error = glz::read_file_json<glz::opts{.error_on_unknown_keys=false}>(test, item.c_str(), buffer);
    std::string descriptive_error = glz::format_error(error, buffer);



    return error.ec == glz::error_code::none;
}

}