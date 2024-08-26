#pragma once

// Shared Includes
#include <ScrewjankShared/DataDefinitions/Assets/AssetType.hpp>
#include <ScrewjankShared/Math/Vec2.hpp>
#include <ScrewjankShared/Math/Vec3.hpp>
#include <ScrewjankShared/Math/VecHash.hpp>

// STD Includes
#include <cstdint>
#include <functional>

namespace sj
{
    struct Vertex
    {
        Vec3 pos;
        Vec3 color;
        Vec2 uv;

        inline bool operator==(const Vertex& other) const
        {
            return pos == other.pos && color == other.color && uv == other.uv;
        }
    };

    struct Model
    {
        AssetType type = AssetType::kModel;
        int32_t numVerts;
        int32_t numIndices;
    };
}

namespace std
{
    template <>
    struct hash<sj::Vertex>
    {
        size_t operator()(sj::Vertex const& vertex) const
        {
            return ((hash<sj::Vec3>()(vertex.pos) ^ (hash<sj::Vec3>()(vertex.color) << 1)) >> 1) ^
                   (hash<sj::Vec2>()(vertex.uv) << 1);
        }
    };
} // namespace std