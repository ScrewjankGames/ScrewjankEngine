export module sj.engine.rendering.materials:DefaultMaterial;
import sj.std.math;
import sj.datadefs.AssetDB;

export namespace sj
{

struct DefaultMaterial
{
    vec4 baseAlbedo {1.0f, 1.0f, 1.0f, 1.0f};
    AssetID albedoTexture = kInvalidAssetID;
};

} // namespace sj