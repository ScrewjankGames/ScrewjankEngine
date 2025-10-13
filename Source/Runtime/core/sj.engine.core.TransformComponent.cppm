module;
#include <ScrewjankEngine/components/ComponentMacros.hpp>
#include <glaze/glaze.hpp>

export module sj.engine.core.TransformComponent;
import sj.datadefs.DataChunk;
import sj.datadefs.components.TransformChunk;
import sj.std.math;
import sj.engine.ecs.ECSRegistry;
import sj.engine.ecs.Identifiers;
import sj.engine.ecs.Serialization;

export namespace sj
{
struct TransformComponent
{
    SJ_COMPONENT(TransformComponent, TransformChunk);

    TransformComponent* parent = nullptr;
    Mat44 localToParentTransform = Mat44(kIdentityTag);
};

template <>
void LoadComponent<TransformComponent>(ECSRegistry& registry,
                                       GameObjectId goId,
                                       const DataChunk& componentData)
{
    auto chunk = componentData.Get<TransformChunk>();

    registry.CreateComponent<TransformComponent>(goId, nullptr, chunk.localToParent);
};
} // namespace sj