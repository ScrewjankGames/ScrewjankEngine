module;
// SJ Includes

// STD Includes
#include <cstdint>
#include <functional>
export module sj.datadefs.assets.Mesh;
export import sj.datadefs.assets.AssetType;
import sj.std.math;

export namespace sj
{
    struct MeshVertex
    {
        Vec3 pos;
        Vec3 normal;
        Vec2 uv;

        inline bool operator==(const MeshVertex& other) const
        {
            return pos == other.pos && normal == other.normal && uv == other.uv;
        }
    };

    struct MeshHeader
    {
        AssetType type = AssetType::kMesh;
        uint8_t indexSize = 0;
        uint32_t numVerts = 0;
        uint32_t numIndices = 0;
    };
} // namespace sj

export namespace std
{
    template <>
    struct hash<sj::MeshVertex>
    {
        size_t operator()(sj::MeshVertex const& vertex) const
        {
            return ((hash<sj::Vec3>()(vertex.pos) ^ (hash<sj::Vec3>()(vertex.normal) << 1)) >> 1) ^
                   (hash<sj::Vec2>()(vertex.uv) << 1);
        }
    };
} // namespace std