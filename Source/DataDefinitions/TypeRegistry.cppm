module;

// SJ Headers
#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/TypeMacros.hpp>

// Library Headers
#include <glaze/glaze.hpp>
#include <glaze/beve/write.hpp>
#include <glaze/core/context.hpp>
#include <glaze/core/meta.hpp>
#include <glaze/json/json_t.hpp>

// STD Headers
#include <array>
#include <cstring>
#include <ranges>
#include <span>
#include <vector>
#include <expected>

export module sj.datadefs:TypeRegistry;
import game.datadefs;
import sj.std.containers;
import :CameraComponent;
import :TransformComponent;
import :Serialization;

export namespace sj
{
    using EngineComponentTypes = type_list<TransformComponent, CameraComponent>;
    using ComponentTypeList = concat_type_lists<EngineComponentTypes, GameComponentTypes>::list;
    constexpr ComponentTypeList g_componentTypes;

    template <class T>
    concept ComponentType = requires(T obj) {
        { T::kTypeId } -> std::convertible_to<sj::TypeId>;
    };

    using AnyComponent = sj::static_any<128, alignof(std::max_align_t)>;

    struct ComponentMetaInfo
    {
        std::string_view m_typeName;
        struct SerializationFuncs
        {
            glz::error_ctx (*toBeveFn)(const glz::json_t& data, sj::dynamic_vector<std::byte>& buffer);
            std::expected<AnyComponent, glz::error_ctx> (*fromBeveFn)(std::span<std::byte> buffer);
        } m_serializationFuncs {};
    };

    template <class T>
    constexpr auto GetComponentMetaInfo() -> ComponentMetaInfo
    {
        auto toBeveFn = [](const glz::json_t& data,
                           sj::dynamic_vector<std::byte>& buffer) -> glz::error_ctx {
            T val;
            glz::error_ctx err = glz::read_json<T>(val, data);
            SJ_ASSERT(err.ec == glz::error_code::none, "Failed to parse component JSON!");

            return glz::write_beve<T>(std::move(val), buffer);
        };

        auto fromBeveFn =
            [](std::span<std::byte> buffer) -> std::expected<AnyComponent, glz::error_ctx> {
            T component;

            glz::error_ctx err = glz::read_beve(component, buffer);
            if(err != glz::error_code::none)
                return std::unexpected(err);

            return component;
        };

        return ComponentMetaInfo {
            .m_typeName = glz::name_v<T>,

            .m_serializationFuncs = {.toBeveFn = toBeveFn, .fromBeveFn = fromBeveFn}};
    }

    constexpr type_map<
        g_componentTypes,
        TypeId,
        ComponentMetaInfo,
        []<class T>() -> TypeId { return T::kTypeId; },
        []<class T>() -> ComponentMetaInfo { return GetComponentMetaInfo<T>(); }>
    g_componentSerializationFuncs;

} // namespace sj
