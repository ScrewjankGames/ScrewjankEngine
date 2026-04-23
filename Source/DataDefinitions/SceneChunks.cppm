module;

// STD Includes
#include <cstdint>
#include <cstddef>

export module sj.datadefs:SceneChunks;
import sj.std.string_hash;
import sj.std.type_info;
import sj.std.containers.vector;
import sj.datadefs.DataChunk;

export namespace sj
{

struct GameObjectChunk
{
    hashed_string_sv id;
    sj::dynamic_vector<DataChunk> components;
};

struct SceneChunk
{
    hashed_string_sv scene_name;
    uint32_t memory; // Free ram allocated to this scene

    sj::dynamic_vector<GameObjectChunk> game_objects;
};

} // namespace sj