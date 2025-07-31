module;

#include <ScrewjankDataDefinitions/ChunkMacros.hpp>

export module sj.datadefs.components.Mesh3DChunk;
import sj.std;

export namespace sj
{
    struct Mesh3DChunk
    {
        SJ_CHUNK(Mesh3DChunk);

        static_string<256> model_path;
        static_string<256> texture_path;

    };
}