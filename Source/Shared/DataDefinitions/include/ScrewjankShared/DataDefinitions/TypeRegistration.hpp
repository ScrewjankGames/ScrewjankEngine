#pragma once

#include <array>
#include <cstring>
#include <glaze/beve/write.hpp>
#include <glaze/core/context.hpp>
#include <glaze/core/meta.hpp>
#include <glaze/glaze.hpp>
#include <glaze/json/json_t.hpp>
#include <ranges>
#include <span>
#include <vector>

#include <ScrewjankShared/string/StringHash.hpp>
#include <ScrewjankShared/utils/Assert.hpp>

import sj.shared.datadefs;

namespace sj 
{
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

    struct ComponentRegistration
    {
        sj::TypeId m_componentTypeId;
        std::string_view m_typeName;
        struct SerializationFuncs
        {
            void (*fromJsonFn)(const glz::json_t::object_t&, void* dest);
            void (*toBeveFn)(std::span<const ComponentSchema> componentList, std::vector<std::byte>& out_buffer);
            //std::function<bool()> toBeve;
            //std::function<bool()> fromBeve;
        } m_serializationFuncs;
    };

    template <class T>
    void GenericToBeve(std::span<const ComponentSchema> componentList, std::vector<std::byte>& out_buffer)
    {
        std::vector<T> components;

        for(const ComponentSchema& schema : componentList)
        {
            T val;
            glz::error_ctx err = glz::read_json(val, schema.componentJson);
            SJ_ASSERT(err.ec == glz::error_code::none, "Failed to parse component json");
            components.emplace_back(std::move(val));
        }

        glz::error_ctx err = glz::write_beve(components[0], out_buffer);
        SJ_ASSERT(err.ec == glz::error_code::none, "Failed to serialize component list");
    }

    template<class T>
    constexpr ComponentRegistration RegisterComponent()
    {
        return ComponentRegistration 
        {
            .m_componentTypeId = T::kTypeId,
            .m_typeName = glz::name_v<T>,
            .m_serializationFuncs = {
                .toBeveFn = GenericToBeve<T>
            }
        };
    }

    constexpr std::span<const ComponentRegistration> RegisterEngineComponents()
    {
        static constexpr std::array s_engineComponents
        {
            RegisterComponent<TransformComponent>(),
            RegisterComponent<CameraComponent>()
        };
        
        return s_engineComponents;
    }

    constexpr decltype(auto) GetComponentRegistry()
    {
        using ComponentRegistrationList = std::span<const ComponentRegistration>;
        static std::array<ComponentRegistrationList, 2> registrations
        {
            RegisterEngineComponents(),
            {}
        };

        return registrations | std::views::join;
    }

    class TypeRegistry
    {
    public:
        static const ComponentRegistration& GetComponentRegistration(sj::TypeId typeId)
        {
            auto registry = GetComponentRegistry();

            auto registrationIt = std::ranges::find(registry, typeId, &sj::ComponentRegistration::m_componentTypeId);
            SJ_ASSERT(registrationIt != registry.end(), "Failed to lookup component type");

            return *registrationIt;
        }
    };
}