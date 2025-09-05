module;
// SJ Includes
#include <ScrewjankDataDefinitions/Assets/AssetType.hpp>

// STD Includes
#include <cstdint>
#include <functional>
export module sj.datadefs.assets.Mesh;
import sj.std.math;

export namespace sj
{
    struct MeshVertex
    {
        Vec3 pos;
        Vec3 color;
        Vec2 uv;

        inline bool operator==(const MeshVertex& other) const
        {
            return pos == other.pos && color == other.color && uv == other.uv;
        }
    };

    struct MeshHeader
    {
        AssetType type = AssetType::kMesh;
        int32_t numVerts = 0;
        int32_t numIndices = 0;
    };
} // namespace sj

export namespace std
{
    template <>
    struct hash<sj::MeshVertex>
    {
        size_t operator()(sj::MeshVertex const& vertex) const
        {
            return ((hash<sj::Vec3>()(vertex.pos) ^ (hash<sj::Vec3>()(vertex.color) << 1)) >> 1) ^
                   (hash<sj::Vec2>()(vertex.uv) << 1);
        }
    };
} // namespace std