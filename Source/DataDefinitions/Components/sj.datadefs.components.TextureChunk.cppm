module;

#include <ScrewjankDataDefinitions/ChunkMacros.hpp>

export module sj.datadefs.components.TextureChunk;
import sj.std.containers.static_string;

export namespace sj
{
    struct TextureChunk
    {
        SJ_CHUNK(TextureChunk)
        static_string<256> path;
    };

} // namespace sj