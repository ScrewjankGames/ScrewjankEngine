module;

// SJ Headers
#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankShared/string/StringHash.hpp>

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
#include <flat_map>
#include <expected>

export module sj.shared.datadefs:TypeRegistry;
import game.datadefs;
import sj.std.containers;
import :CameraComponent;
import :TransformComponent;
import :Serialization;

export namespace sj
{
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
            glz::error_ctx (*toBeveFn)(const glz::json_t& data,
                                       sj::dynamic_vector<std::byte>& buffer);
            std::expected<AnyComponent, glz::error_ctx> (*fromBeveFn)(std::span<std::byte> buffer);
        } m_serializationFuncs {};
    };

    template <class T>
    constexpr ComponentMetaInfo GetComponentMetaInfo()
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

    inline constexpr type_list<TransformComponent, CameraComponent> g_engineComponentTypes;

    class ComponentTypeRegistry
    {
    public:
        static constexpr size_t kNumComponentTypes = g_engineComponentTypes.size() + g_gameComponentTypes.size();

        template <auto F, class... Args>
        static constexpr void ForEachComponentType(Args&&... args)
        {
            g_engineComponentTypes.for_each<F>(std::forward<Args>(args)...);
            g_gameComponentTypes.for_each<F>(std::forward<Args>(args)...);
        }

        using ComponentLookupEntry = std::pair<TypeId, ComponentMetaInfo>;
        using ComponentInfoLookupList =
            std::array<ComponentLookupEntry,
                       g_engineComponentTypes.size() + g_gameComponentTypes.size()>;
        static constexpr ComponentInfoLookupList GetMetaInfoTable()
        {
            static constexpr ComponentInfoLookupList kLookupList = []() {
                ComponentInfoLookupList list;

                size_t i = 0;
                auto helper = []<class T>(size_t& i, ComponentInfoLookupList& list) {
                    list.at(i++) = std::pair {T::kTypeId, GetComponentMetaInfo<T>()};
                };

                ForEachComponentType<helper>(i, list);

                return list;
            }();

            return kLookupList;
        }


        static constexpr const ComponentMetaInfo* FindComponentMetaInfo(TypeId typeId)
        {
            static constexpr ComponentInfoLookupList metaInfoTable = GetMetaInfoTable();

            const auto& it = std::ranges::find(metaInfoTable, typeId, &ComponentLookupEntry::first);

            if(it == metaInfoTable.end())
            {
                return nullptr;
            }
            else
            {
                return &(it->second);
            }
        }
    };
} // namespace sj