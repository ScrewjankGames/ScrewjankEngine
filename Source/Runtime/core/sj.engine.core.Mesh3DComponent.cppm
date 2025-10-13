module;
#include <ScrewjankEngine/components/ComponentMacros.hpp>

export module sj.engine.core.Mesh3DComponent;
import sj.datadefs.DataChunk;
import sj.datadefs.components.Mesh3DChunk;
import sj.std.math;
import sj.engine.ecs.ECSRegistry;
import sj.engine.ecs.Identifiers;
import sj.engine.ecs.Serialization;

export namespace sj
{
struct Mesh3DComponent
{
    SJ_COMPONENT(Mesh3DComponent, Mesh3DChunk);
};

template <>
void LoadComponent<Mesh3DComponent>(ECSRegistry& registry,
                                    GameObjectId goId,
                                    const DataChunk& componentData)
{
    auto chunk = componentData.Get<Mesh3DChunk>();

    registry.CreateComponent<Mesh3DComponent>(goId);
};
} // namespace sj