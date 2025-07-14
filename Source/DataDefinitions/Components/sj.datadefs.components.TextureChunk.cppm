module;

#include <ScrewjankDataDefinitions/ChunkMacros.hpp>
#include <array>

export module sj.datadefs.components.TextureChunk;
import sj.std.math;

export namespace sj
{
    struct TextureChunk
    {
        SJ_CHUNK(TextureChunk)

        std::array<char, 256> path;
    };

} // namespace sj