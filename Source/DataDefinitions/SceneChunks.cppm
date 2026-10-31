module;

#include <glaze/glaze.hpp>

#include <cstdint>
#include <cstddef>
#include <utility>

export module sj.datadefs:SceneChunks;
import sj.std.primitives;
import sj.std.string_hash;
import sj.std.type_info;
import sj.std.containers.vector;

export namespace sj
{
using ComponentChunk = std::pair<hashed_string_sv, glz::generic_u64>;

struct GameObjectChunk
{
    hashed_string_sv id;
    sj::dynamic_vector<ComponentChunk> components;
    sj::dynamic_vector<GameObjectChunk> children;
};

struct SceneChunk
{
    hashed_string_sv scene_name;

    sj::dynamic_vector<GameObjectChunk> game_objects;
};

struct SceneChunkHeader
{
    hashed_string_sv scene_name;
    u32 num_game_objects = 0;
};

struct GameObjectChunkHeader
{
    hashed_string_sv name;
    u32 num_components = 0;
    u32 num_children = 0;
};

struct ComponentChunkHeader
{
    std::span<const TypeId> load_chunk_type_ids;
};

} // namespace sj