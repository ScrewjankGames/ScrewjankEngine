module;

#include <ScrewjankStd/Assert.hpp>

#include <limits>
#include <filesystem>
#include <fstream>
#include <ios>
#include <span>

export module sj.builders:MeshUtils;
import sj.datadefs;

export namespace sj::build
{
void WriteMeshToFile(std::vector<MeshVertex> verts,
                     std::vector<MeshIndexType> indices,
                     const std::filesystem::path& outputFilePath)
{
    const size_t vertexMemSize = (sizeof(MeshVertex) * verts.size());
    const size_t indexMemSize = (sizeof(MeshIndexType) * indices.size());

    MeshHeader mesh {};
    mesh.type = AssetType::kMesh;
    mesh.indexSize = sizeof(MeshIndexType);

    SJ_ASSERT(verts.size() <= std::numeric_limits<decltype(MeshHeader::numVerts)>::max(),
              "Too many vertices to fit in Mesh::NumVerts");
    mesh.numVerts = static_cast<decltype(MeshHeader::numVerts)>(verts.size());

    SJ_ASSERT(indices.size() <= std::numeric_limits<decltype(MeshHeader::numIndices)>::max(),
              "Too many vertices to fit in Mesh::NumVerts");
    mesh.numIndices = static_cast<decltype(MeshHeader::numIndices)>(indices.size());

    std::ofstream outputFile;
    outputFile.open(outputFilePath, std::ios::out | std::ios::binary);
    SJ_ASSERT(outputFile.is_open(), "Failed to open output file {}", outputFilePath.c_str());
    outputFile.write(reinterpret_cast<char*>(&mesh), sizeof(mesh));

    SJ_ASSERT(vertexMemSize < std::numeric_limits<std::streamsize>::max(),
              "Vertex blob too big for single write!");
    SJ_ASSERT(indexMemSize < std::numeric_limits<std::streamsize>::max(),
              "Index blob too big for single write!");

    outputFile.write(reinterpret_cast<char*>(verts.data()),
                     static_cast<std::streamsize>(vertexMemSize));
    outputFile.write(reinterpret_cast<char*>(indices.data()),
                     static_cast<std::streamsize>(indexMemSize));

    outputFile.close();
}
} // namespace sj::build