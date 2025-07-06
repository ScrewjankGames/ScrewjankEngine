module;

#include <ScrewjankDataDefinitions/ChunkMacros.hpp>

export module sj.datadefs.components.TransformChunk;
import sj.std.math;
import sj.std.string_hash;

export namespace sj
{
    struct TransformChunk
    {
        SJ_CHUNK(TransformChunk);

        string_hash parentId;
        Mat44 localToParent;
    };
} // namespace sj