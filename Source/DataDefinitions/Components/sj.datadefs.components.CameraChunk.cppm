module;

#include <ScrewjankDataDefinitions/ChunkMacros.hpp>

export module sj.datadefs.components.CameraChunk;
import sj.std.math;

export namespace sj
{
    struct CameraChunk
    {
        SJ_CHUNK(CameraChunk);

        Mat44 localToGoTransform;
        float fov = 0;
        float nearPlane = 0;
        float farPlane = 0;
    };
}