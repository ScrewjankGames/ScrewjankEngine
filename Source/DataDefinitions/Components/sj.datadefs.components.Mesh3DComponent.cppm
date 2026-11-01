module;

export module sj.datadefs.components.Mesh3DComponent;
import sj.datadefs.assets.AssetID;

export namespace sj
{
struct Mesh3DComponent
{
    AssetID model_id;
    AssetID texture_id;
};
} // namespace sj