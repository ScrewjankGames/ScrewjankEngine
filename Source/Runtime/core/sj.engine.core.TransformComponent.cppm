module;
#include <glaze/glaze.hpp>

#include <ScrewjankStd/Assert.hpp>

export module sj.engine.core.TransformComponent;
import sj.datadefs.DataChunk;
import sj.std.math;
import sj.engine.ecs.ECSRegistry;
import sj.engine.ecs.Identifiers;
import sj.engine.ecs.Serialization;

export namespace sj
{
struct TransformComponent
{
    TransformComponent* parent = nullptr;
    Mat44 localToParentTransform = Mat44(kIdentityTag);
};

struct TransformChunk
{
    Mat44 localToParent;
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