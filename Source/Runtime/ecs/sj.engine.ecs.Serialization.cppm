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
    registry.CreateComponent<T>(goId, componentData.Get<T>());
};
} // namespace sj