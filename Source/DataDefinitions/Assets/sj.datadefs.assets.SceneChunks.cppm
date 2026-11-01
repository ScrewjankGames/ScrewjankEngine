module;

#include <glaze/glaze.hpp>

#include <cstdint>
#include <cstddef>
#include <utility>

export module sj.datadefs.assets.SceneChunks;
import sj.std.string_hash;
import sj.std.primitives;
import sj.std.type_info;

export namespace sj
{
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