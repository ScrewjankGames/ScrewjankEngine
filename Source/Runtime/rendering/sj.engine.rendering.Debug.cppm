module;

#include <SDL3/SDL_gpu.h>

#include <span>

export module sj.engine.rendering.Debug;

import sj.engine.rendering.pipelines;
import sj.std;

#ifndef SJ_GOLD

export namespace sj::debug
{

inline constexpr uZ kMaxDebugVerts = 2048;
inline constexpr uZ kMaxDebugIndices = 4096;
inline constexpr uZ kDebugPrimBufferSize =
    (sizeof(DebugVertex) * kMaxDebugVerts) + (sizeof(u32) * kMaxDebugIndices);

void InitDebugDraw(sj::memory_resource* resource);
void DrawLine(const vec4& start, const vec4& end, const color& c);

struct DebugDrawCPUData
{
    struct DebugPrim
    {
        u32 verticesIndexOffset = 0;
        u32 indicesIndexOffset = 0;
        u32 numIndices = 0;
    };

    std::span<DebugVertex> verts;
    std::span<u32> indices;
    std::span<DebugPrim> prims;

    u32 vertexBufferSizeBytes = 0;
    u32 indexBufferSizeBytes = 0;
};
DebugDrawCPUData GetDebugDrawCPUData();
void ClearDebugDrawCPUBuffers();

} // namespace sj::debug

#endif