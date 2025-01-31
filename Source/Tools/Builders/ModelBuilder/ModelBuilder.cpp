// Shared Includes
#include <ScrewjankShared/io/File.hpp>
#include <ScrewjankShared/DataDefinitions/Assets/Model.hpp>
#include <ScrewjankShared/utils/Assert.hpp>

// Library Includes
#include <tiny_obj_loader.h>

// STD Includes
#include <new>
#include <cstdio>
#include <unordered_map>
#include <limits>

using namespace sj;

void ExtractBuffers(const char* inputFilePath,
                    std::vector<Vertex>& out_verts,
                    std::vector<uint16_t>& out_indices)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inputFilePath);
    SJ_ASSERT(success,
              "Failed to load model %s.\n warn: %s\n err: %s",
              inputFilePath,
              warn.c_str(),
              err.c_str());

    out_verts.reserve(attrib.vertices.size() / 3);
    out_indices.reserve(out_verts.capacity());

    std::unordered_map<Vertex, uint16_t> uniqueVertices {};

    for(const tinyobj::shape_t& shape : shapes)
    {
        for(const tinyobj::index_t& index : shape.mesh.indices)
        {
            Vertex vertex {};

            vertex.pos = {attrib.vertices[3 * index.vertex_index + 0],
                          attrib.vertices[3 * index.vertex_index + 1],
                          attrib.vertices[3 * index.vertex_index + 2]};

            vertex.uv = {attrib.texcoords[2 * index.texcoord_index + 0],
                         1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

            vertex.color = {1.0f, 1.0f, 1.0f};

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

int main(int argc, char** argv)
{
    const char* inputFilePath = argv[1];
    const char* outputFilePath = argv[2];

    std::vector<Vertex> verts;
    std::vector<uint16_t> indices;
    ExtractBuffers(inputFilePath, verts, indices);

    const size_t vertexMemSize = (sizeof(Vertex) * verts.size());
    const size_t indexMemSize = (sizeof(uint16_t) * indices.size());

    void* modelMem = malloc(
        sizeof(Model) + 
        vertexMemSize + indexMemSize
    );

    Model* model = new(modelMem) Model {};
    model->type = AssetType::kModel;

    SJ_ASSERT(verts.size() <= std::numeric_limits<decltype(Model::numVerts)>::max(),
              "Too many vertices to fit in Model::NumVerts");
    model->numVerts = static_cast<decltype(Model::numVerts)>(verts.size());

    SJ_ASSERT(indices.size() <= std::numeric_limits<decltype(Model::numIndices)>::max(),
            "Too many vertices to fit in Model::NumVerts");
    model->numIndices = static_cast<decltype(Model::numIndices)>(indices.size());

    File outputFile;
    outputFile.Open(outputFilePath, File::OpenMode::kWriteBinary);

    outputFile.Write(model, sizeof(Model));

    outputFile.Write(verts.data(), vertexMemSize);
    outputFile.Write(indices.data(), indexMemSize);

    outputFile.Close();

    free(modelMem);
    return 0;
}