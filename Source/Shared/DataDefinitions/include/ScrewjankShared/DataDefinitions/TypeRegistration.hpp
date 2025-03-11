#pragma once

#include <ScrewjankShared/string/StringHash.hpp>
#include <array>
#include <cstring>
#include <functional>
#include <glaze/core/context.hpp>
#include <glaze/glaze.hpp>
#include <glaze/json/json_t.hpp>
#include <ranges>
#include <ScrewjankShared/utils/Assert.hpp>

#include <ScrewjankShared/DataDefinitions/Components/CameraComponent.hpp>
#include <ScrewjankShared/DataDefinitions/Components/TransformComponent.hpp>

namespace sj 
{
    struct ComponentData
    {
        std::vector<uint8_t> blob;
    };

    struct ComponentRegistration
    {
        sj::TypeId m_componentTypeId;
        struct SerializationFuncs
        {
            void (*fromJsonFn)(const glz::json_t::object_t&, ComponentData&);
            //std::function<bool()> toBeve;
            //std::function<bool()> fromBeve;
        } m_serializationFuncs;
    };

    template <class T>
    void GenericFromJson(const glz::json_t::object_t& json, ComponentData& out_dataBuffer)
    {
        T val;
        glz::error_ctx err = glz::read<glz::opts{.error_on_missing_keys=false}, T>(val, json);
        SJ_ASSERT(err.ec == glz::error_code::none, "Error deserializing Component data");
        out_dataBuffer.blob.resize(sizeof(T));
        memcpy(out_dataBuffer.blob.data(), &val, sizeof(T));
    }

    template<class T>
    constexpr ComponentRegistration RegisterComponent()
    {
        ComponentRegistration registration{};
        registration.m_componentTypeId = T::kTypeId;
        registration.m_serializationFuncs.fromJsonFn = GenericFromJson<T>;
        return registration;
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
        std::array<ComponentRegistrationList, 2> registrations;
        registrations[0] = RegisterEngineComponents(); 

        return registrations | std::views::join;
    }
}