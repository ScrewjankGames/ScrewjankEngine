module;

#include <ScrewjankStd/Assert.hpp>

#include <algorithm>
#include <cstring>
#include <span>
#include <vector>
#include <numbers>

module sj.engine.debug.DebugDraw;
import sj.engine.system.memory.MemorySystem;
import sj.std;

#ifndef SJ_GOLD
namespace sj::debug
{

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
static std::pmr::vector<DebugVertex> gDebugVertexBuffer {};
static std::pmr::vector<u32> gDebugIndexBuffer {};
static std::pmr::vector<DebugDrawCPUData::DebugPrim> gDebugPrims;
static bool gInitialized = false;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

void InitDebugDraw(sj::memory_resource* resource)
{
    SJ_ASSERT(!gInitialized, "Don't double initialize the debug drawing api");
    std::destroy_at(&gDebugVertexBuffer);
    std::construct_at(&gDebugVertexBuffer, resource);

    std::destroy_at(&gDebugIndexBuffer);
    std::construct_at(&gDebugIndexBuffer, resource);

    std::destroy_at(&gDebugPrims);
    std::construct_at(&gDebugPrims, resource);
    gInitialized = true;
}

void DrawLine(const vec4& start, const vec4& end, const color& c)
{
    SJ_ASSERT(gInitialized, "Please initialize the debug drawing api");

    u32 vertexOffset = static_cast<u32>(gDebugVertexBuffer.size());
    gDebugVertexBuffer.emplace_back(DebugVertex {
        .pos = vec3(start.get_x(), start.get_y(), start.get_z()),
        .color = vec3(c.r, c.g, c.b),
    });
    gDebugVertexBuffer.emplace_back(DebugVertex {
        .pos = vec3(end.get_x(), end.get_y(), end.get_z()),
        .color = vec3(c.r, c.g, c.b),
    });

    u32 indexOffset = static_cast<u32>(gDebugIndexBuffer.size());
    gDebugIndexBuffer.emplace_back(0);
    gDebugIndexBuffer.emplace_back(1);

    gDebugPrims.emplace_back(vertexOffset, indexOffset, 2);
}

void DrawMatrix(const mat44& m)
{
    DrawLine(m.get_w(), m.get_w() + m.get_x(), sj::colors::red);
    DrawLine(m.get_w(), m.get_w() + m.get_y(), sj::colors::green);
    DrawLine(m.get_w(), m.get_w() - m.get_z(), sj::colors::blue);
}

DebugDrawCPUData GetDebugDrawCPUData()
{
    return DebugDrawCPUData {
        .verts = gDebugVertexBuffer,
        .indices = gDebugIndexBuffer,
        .prims = gDebugPrims,
        .vertexBufferSizeBytes =
            static_cast<u32>(std::as_bytes(std::span {gDebugVertexBuffer}).size()),
        .indexBufferSizeBytes =
            static_cast<u32>(std::as_bytes(std::span {gDebugIndexBuffer}).size()),
    };
}

void ClearDebugDrawCPUBuffers()
{
    gDebugVertexBuffer.clear();
    gDebugIndexBuffer.clear();
    gDebugPrims.clear();
}

} // namespace sj::debug
#endif