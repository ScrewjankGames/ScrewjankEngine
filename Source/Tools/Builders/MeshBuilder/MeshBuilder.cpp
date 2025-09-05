// Parent Include
#include "MeshBuilder.hpp"

// SJ Includes
#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankDataDefinitions/Assets/AssetType.hpp>

// Library Includes
#include <tiny_obj_loader.h>

// STD Includes
#include <cstdio>
#include <unordered_map>
#include <limits>
#include <fstream>

import sj.datadefs.assets.Mesh;

namespace sj::build
{
void ExtractBuffers(const char* inputFilePath,
                    std::vector<MeshVertex>& out_verts,
                    std::vector<uint16_t>& out_indices)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inputFilePath);
    (void)success;
    // SJ_ASSERT(success,
    //           "Failed to load mesh {}.\n warn: {}\n err: {}",
    //           inputFilePath,
    //           warn.c_str(),
    //           err.c_str());

    out_verts.reserve(attrib.vertices.size() / 3);
    out_indices.reserve(out_verts.capacity());

    std::unordered_map<MeshVertex, uint16_t> uniqueVertices {};

    for(const tinyobj::shape_t& shape : shapes)
    {
        for(const tinyobj::index_t& index : shape.mesh.indices)
        {
            MeshVertex vertex {};

            vertex.pos = {.x=attrib.vertices[3 * index.vertex_index + 0],
                          .y=attrib.vertices[3 * index.vertex_index + 1],
                          .z=attrib.vertices[3 * index.vertex_index + 2]};

            vertex.uv = {attrib.texcoords[2 * index.texcoord_index + 0],
                         1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

            vertex.color = {.x=1.0f, .y=1.0f, .z=1.0f};

            if(uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint16_t>(out_verts.size());
                out_verts.push_back(vertex);
            }

            out_indices.push_back(uniqueVertices[vertex]);
        }
    }
    SJ_ASSERT(out_indices.size() < std::numeric_limits<uint16_t>::max(),
              "Index count out of range of uint16 for index buffers")
}

bool MeshBuilder::BuildItem(const std::filesystem::path& item, const std::filesystem::path& output_path) const 
{
    std::vector<MeshVertex> verts;
    std::vector<uint16_t> indices;
    ExtractBuffers(item.c_str(), verts, indices);

    const size_t vertexMemSize = (sizeof(MeshVertex) * verts.size());
    const size_t indexMemSize = (sizeof(uint16_t) * indices.size());
    
    MeshHeader mesh{};
    mesh.type = AssetType::kMesh;

    SJ_ASSERT(verts.size() <= std::numeric_limits<decltype(MeshHeader::numVerts)>::max(),
              "Too many vertices to fit in Mesh::NumVerts");
    mesh.numVerts = static_cast<decltype(MeshHeader::numVerts)>(verts.size());

    SJ_ASSERT(indices.size() <= std::numeric_limits<decltype(MeshHeader::numIndices)>::max(),
            "Too many vertices to fit in Mesh::NumVerts");
    mesh.numIndices = static_cast<decltype(MeshHeader::numIndices)>(indices.size());

    std::ofstream outputFile;
    outputFile.open(output_path, std::ios::out | std::ios::binary);
    SJ_ASSERT(outputFile.is_open(), "Failed to open output file {}", output_path.c_str());
    outputFile.write(reinterpret_cast<char*>(&mesh), sizeof(mesh));

    outputFile.write(reinterpret_cast<char*>(verts.data()), vertexMemSize);
    outputFile.write(reinterpret_cast<char*>(indices.data()), indexMemSize);

    outputFile.close();

    return true;
}

}
