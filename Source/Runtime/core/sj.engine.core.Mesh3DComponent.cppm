module;

#include <string_view>

export module sj.engine.core.Mesh3DComponent;
import sj.datadefs.DataChunk;
import sj.std.math;
import sj.engine.ecs.ECSRegistry;
import sj.engine.ecs.Identifiers;
import sj.engine.ecs.Serialization;

export namespace sj
{
struct Mesh3DComponent
{
};

struct Mesh3DChunk
{
    std::string_view model_path;
    std::string_view texture_path;
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