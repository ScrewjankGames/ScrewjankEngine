module;

#include <ScrewjankStd/TypeMacros.hpp>

// STD Includes
#include <cstdint>
#include <cstddef>

export module sj.datadefs:SceneChunks;
import sj.std.containers.vector;
import sj.datadefs.DataChunk;

export namespace sj
{
    struct GameObjectChunk
    {
        string_hash id;
        sj::dynamic_vector<DataChunk> components;
    };

    struct SceneChunk
    {
        string_hash name;
        uint32_t memory; // Free ram allocated to this scene

        sj::dynamic_vector<GameObjectChunk> gameObjects;
    };
}