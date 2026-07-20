module;

// SJ Includes
#include <ScrewjankStd/Assert.hpp>

// Library Includes
#include <tiny_obj_loader.h>

// STD Includes
#include <cstdio>
#include <unordered_map>
#include <fstream>
#include <span>
#include <filesystem>

export module sj.MeshBuilder;
import sj.std.primitives;
import sj.builders;
import sj.datadefs.assets;

namespace sj::build
{
void ExtractBuffers(const char* inputFilePath,
                    std::vector<MeshVertex>& out_verts,
                    std::vector<MeshIndexType>& out_indices);
} // namespace sj::build

export namespace sj::build
{
class MeshBuilder final : public IBuilder
{

public:
    [[nodiscard]] std::span<const std::string_view> GetExtensions() const override
    {
        using namespace std::literals;
        static constexpr std::array<std::string_view, 1> extensions = {".obj"sv};
        return extensions;
    }

    [[nodiscard]] std::string_view GetBuilderName() const override
    {
        return "Mesh Builder";
    }

    bool BuildItem(BuildContext& in_ctx, const std::filesystem::path& item) override
    {
        auto&& [output_path, id] =
            in_ctx.Import(item, item.filename().replace_extension(".sj_mesh"));

        std::vector<MeshVertex> verts;
        std::vector<MeshIndexType> indices;
        ExtractBuffers(item.c_str(), verts, indices);
        WriteMeshToFile(verts, indices, output_path);
        return true;
    }
};
} // namespace sj::build

namespace sj::build
{
void ExtractBuffers(const char* inputFilePath,
                    std::vector<MeshVertex>& out_verts,
                    std::vector<MeshIndexType>& out_indices)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    [[maybe_unused]] bool success =
        tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inputFilePath);
    SJ_ASSERT(success,
              "Failed to load mesh {}.\n warn: {}\n err: {}",
              inputFilePath,
              warn.c_str(),
              err.c_str());

    out_verts.reserve(attrib.vertices.size() / 3);
    out_indices.reserve(out_verts.capacity());

    std::unordered_map<MeshVertex, MeshIndexType> uniqueVertices {};

    for(const tinyobj::shape_t& shape : shapes)
    {
        for(const tinyobj::index_t& index : shape.mesh.indices)
        {
            MeshVertex vertex {};

            vertex.pos = {.x = attrib.vertices[3 * index.vertex_index + 0],
                          .y = attrib.vertices[3 * index.vertex_index + 1],
                          .z = attrib.vertices[3 * index.vertex_index + 2]};

            vertex.uv = {attrib.texcoords[2 * index.texcoord_index + 0],
                         attrib.texcoords[2 * index.texcoord_index + 1]};

            vertex.normal = {.x = attrib.normals[3 * index.normal_index + 0],
                             .y = attrib.normals[3 * index.normal_index + 1],
                             .z = attrib.normals[3 * index.normal_index + 2]};

            if(uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<MeshIndexType>(out_verts.size());
                out_verts.push_back(vertex);
            }

            out_indices.push_back(uniqueVertices[vertex]);
        }
    }
    SJ_ASSERT(out_indices.size() < std::numeric_limits<u32>::max(),
              "Index count out of range of uint32 for index buffers")
}
} // namespace sj::build
