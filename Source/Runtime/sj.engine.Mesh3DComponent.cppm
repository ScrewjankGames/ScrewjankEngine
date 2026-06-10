module;

export module sj.engine.Mesh3DComponent;
import sj.datadefs;
import sj.std.math;
import sj.engine.ecs.ECSRegistry;
import sj.engine.ecs.Identifiers;
import sj.engine.ecs.Serialization;

export namespace sj
{
struct Mesh3DComponent
{
    AssetID model_id;
    AssetID texture_id;
};
} // namespace sj