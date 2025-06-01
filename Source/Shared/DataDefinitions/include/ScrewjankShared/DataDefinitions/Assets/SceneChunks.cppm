module;

#include <ScrewjankShared/string/StringHash.hpp>

// STD Includes
#include <cstdint>
#include <cstddef>

export module sj.shared.datadefs:SceneChunks;
import sj.std.containers;

export namespace sj
{
    // Chunk as in "chunk of data"
    struct AnyChunk
    {
        sj::TypeId type;
        sj::dynamic_vector<std::byte> data;
    };

    struct GameObjectChunk
    {
        StringHash id;
        sj::dynamic_vector<AnyChunk> components;
    };

    struct SceneChunk
    {
        StringHash name;
        uint32_t memory; // Free ram allocated to this scene

        sj::dynamic_vector<GameObjectChunk> gameObjects;
    };
}