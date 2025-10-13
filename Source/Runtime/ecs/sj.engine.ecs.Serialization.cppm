module;

#include <ScrewjankStd/Assert.hpp>

#include <glaze/glaze.hpp>

#include <expected>

export module sj.engine.ecs.Serialization;
import sj.datadefs;

import sj.engine.ecs.ECSRegistry;
import sj.engine.ecs.Identifiers;

export namespace sj
{
using LoadComponentFn = void (*)(ECSRegistry& registry,
                                 GameObjectId goId,
                                 const DataChunk& componentData);

template <class T>
void LoadComponent(ECSRegistry& registry, GameObjectId goId, const DataChunk& componentData)
{
    T component =
        glz::read_beve<T>(componentData.data)
            .or_else([](const glz::error_ctx& error) -> std::expected<T, glz::error_ctx> {
                std::string_view componentTypeName = glz::type_name<T>;

                SJ_ASSERT(error.ec == glz::error_code::none,
                          "Failed to translate from component chunk to runtime component type {}!",
                          componentTypeName);
                return T();
            })
            .value();

    registry.CreateComponent<T>(goId, std::move(component));
};
} // namespace sj