module;

#include <glaze/glaze.hpp>

#include <cstdint>
#include <cstddef>
#include <utility>

export module sj.datadefs:SceneChunks;
import sj.std.string_hash;
import sj.std.type_info;
import sj.std.containers.vector;
import sj.datadefs.DataChunk;

export namespace sj
{
using ComponentChunk = std::pair<hashed_string_sv, glz::generic_u64>;

struct GameObjectChunk
{
    hashed_string_sv id;
    sj::dynamic_vector<ComponentChunk> components;
};

struct SceneChunk
{
    hashed_string_sv scene_name;
    uint32_t memory; // Free ram allocated to this scene

    sj::dynamic_vector<GameObjectChunk> game_objects;
};

} // namespace sj