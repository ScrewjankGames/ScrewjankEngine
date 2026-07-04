module;

// SJ Includes
#include <ScrewjankStd/Assert.hpp>

// Library Includes
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>

// STD Includes
#include <filesystem>
#include <format>
#include <string_view>
#include <flat_map>
#include <span>

export module sj.build.GlbBuilder;
import sj.TextureBuilder;

import sj.builders;
import sj.datadefs;
import sj.std;

namespace stdfs = std::filesystem;

export namespace sj::build
{
class GlbBuilder final : public IBuilder
{
public:
    [[nodiscard]] std::span<const std::string_view> GetExtensions() const override
    {
        using namespace std::literals;
        static constexpr std::array<std::string_view, 1> extensions = {".glb"sv};
        return extensions;
    }

    [[nodiscard]] std::string_view GetBuilderName() const override
    {
        return "GLB Builder";
    }

    [[nodiscard]] std::string_view GetOutputExtension() const override
    {
        return ".sj_???";
    }

    bool BuildItem(BuildContext& in_ctx, const stdfs::path& item) override
    {
        auto gltfFile = fastgltf::MappedGltfFile::FromPath(item);
        if(!gltfFile)
        {
            SJ_ENGINE_LOG_ERROR("Failed to open glTF file: {}",
                                fastgltf::getErrorMessage(gltfFile.error()));
            return false;
        }
        constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember
                                     | fastgltf::Options::AllowDouble
                                     | fastgltf::Options::LoadExternalBuffers
                                     | fastgltf::Options::LoadExternalImages
                                     | fastgltf::Options::GenerateMeshIndices;
        fastgltf::Parser parser;
        auto asset = parser.loadGltf(gltfFile.get(), item.parent_path(), gltfOptions);

        if(asset.error() != fastgltf::Error::None)
        {
            SJ_ENGINE_LOG_ERROR("Failed to load glTF: {}",
                                fastgltf::getErrorMessage(asset.error()));
            return false;
        }

        std::flat_map<uZ, AssetID> imageIdxToAssetId;
        for(auto&& [imageIndex, image] : std::views::enumerate(asset->images))
        {
            auto&& [imageOutputPath, assetId] = in_ctx.Import(item, image.name + ".sj_tex");
            BuildImage(asset.get(), image, imageOutputPath);
            imageIdxToAssetId[imageIndex] = assetId;
        }

        for(fastgltf::Mesh& mesh : asset->meshes)
        {
            // Primitives correspond to single drawable thing
            for(auto&& [primIdx, prim] : std::views::enumerate(mesh.primitives))
            {
                std::string primName = std::format("{}_{}", mesh.name, primIdx);
                auto&& [primOutputPath, assetId] = in_ctx.Import(item, primName + ".sj_tex");
                BuildMesh(asset.get(), prim, primOutputPath);
            }
        }

        for(fastgltf::Scene& scene : asset->scenes)
        {
            std::string sceneName = item.string();
            SceneChunk chunk {
                .scene_name = std::string_view(sceneName),
            };

            for(auto nodeIdx : scene.nodeIndices)
            {
                fastgltf::Node& node = asset->nodes[nodeIdx];
                if(node.meshIndex)
                {
                    fastgltf::Mesh& mesh = asset->meshes[*node.meshIndex];
                    for(fastgltf::Primitive& p : mesh.primitives)
                    {
                    }
                }
            }
        }

        return true;
    }

private:
    void
    BuildMesh(fastgltf::Asset& asset, fastgltf::Primitive& meshPrim, const stdfs::path& outputPath)
    {
        const uZ baseColorTexcoordIndex = [&] -> uZ {
            if(!meshPrim.materialIndex.has_value())
                return -1;

            auto& material = asset.materials[meshPrim.materialIndex.value()];
            auto& baseColorTexture = material.pbrData.baseColorTexture;
            if(!baseColorTexture.has_value())
                return -1;

            auto&& baseColorTextureTransform = baseColorTexture->transform;
            if(baseColorTextureTransform && baseColorTextureTransform->texCoordIndex.has_value())
                return baseColorTextureTransform->texCoordIndex.value();
            else
                return baseColorTexture->texCoordIndex;
        }();

        auto* positionAttrIt = meshPrim.findAttribute("POSITION");
        SJ_ASSERT(positionAttrIt != meshPrim.attributes.end(),
                  "A mesh primitive is required to hold the POSITION attribute.");

        auto& positionAccessor = asset.accessors[positionAttrIt->accessorIndex];
        const auto positions = [&] -> std::vector<Vec3> {
            std::vector<Vec3> res;
            res.reserve(positionAccessor.count);
            fastgltf::iterateAccessor<fastgltf::math::fvec3>(
                asset,
                positionAccessor,
                [&](fastgltf::math::fvec3 pos) {
                    res.emplace_back(Vec3(pos.x(), pos.y(), pos.z()));
                });
            return res;
        }();
        const uZ vertexCount = positions.size();

        const auto normals = [&] -> std::vector<Vec3> {
            std::vector<Vec3> res;
            auto* normalsAttrIt = meshPrim.findAttribute("NORMAL");
            if(normalsAttrIt == meshPrim.attributes.end())
            {
                res.resize(vertexCount);
                return res;
            }

            auto& normalsAccessor = asset.accessors[normalsAttrIt->accessorIndex];
            res.reserve(positionAccessor.count);

            fastgltf::iterateAccessor<fastgltf::math::fvec3>(
                asset,
                positionAccessor,
                [&](fastgltf::math::fvec3 pos) {
                    res.emplace_back(Vec3(pos.x(), pos.y(), pos.z()));
                });
            return res;
        }();

        const auto uvs = [&] -> std::vector<Vec2> {
            std::vector<Vec2> res;
            std::string attrName = std::format("TEXCOORD_{}", baseColorTexcoordIndex);
            auto texcoordAttrIt = meshPrim.findAttribute(attrName);
            if(texcoordAttrIt == meshPrim.attributes.end())
            {
                res.resize(vertexCount);
                return res;
            }

            auto& texCoordAccessor = asset.accessors[texcoordAttrIt->accessorIndex];
            res.reserve(texCoordAccessor.count);
            fastgltf::iterateAccessor<fastgltf::math::fvec2>(asset,
                                                             texCoordAccessor,
                                                             [&](fastgltf::math::fvec2 uv) {
                                                                 res.emplace_back(uv.x(), 1.0 - uv.y());
                                                             });
            return res;
        }();

        std::vector<MeshVertex> vertexBuffer = std::views::zip(positions, normals, uvs)
                                               | std::views::transform([](auto&& z) {
                                                     return std::make_from_tuple<MeshVertex>(z);
                                                 })
                                               | std::ranges::to<std::vector>();

        SJ_ASSERT(meshPrim.indicesAccessor.has_value(), "Builder requires indexed primitives");
        auto& indexAccessor = asset.accessors[meshPrim.indicesAccessor.value()];
        std::vector<MeshIndexType> indexBuffer(indexAccessor.count);
        fastgltf::copyFromAccessor<u32>(asset, indexAccessor, indexBuffer.data());

        WriteMeshToFile(vertexBuffer, indexBuffer, outputPath);
    }

    void BuildImage(fastgltf::Asset& asset, fastgltf::Image& image, stdfs::path outputPath)
    {
        auto errorHandlerFn = [](auto& arg) {
            SJ_ASSERT(false, "unhandled image souce");
        };

        std::visit(
            fastgltf::visitor {
                errorHandlerFn,
                [&](fastgltf::sources::URI& filePath) {
                    SJ_ASSERT(filePath.fileByteOffset == 0, "We don't support offsets with stbi.");
                    SJ_ASSERT(filePath.uri.isLocalPath(),
                              "We're only capable of loading local files.");

                    stdfs::path texturePath = filePath.uri.fspath();
                    BuildTextureFromFile(texturePath, outputPath);
                },
                [&](fastgltf::sources::Array& vector) {
                    BuildTextureFromMemory(vector.bytes, outputPath);
                },
                [&](fastgltf::sources::BufferView& view) {
                    auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                    auto& buffer = asset.buffers[bufferView.bufferIndex];
                    std::visit(
                        fastgltf::visitor {
                            errorHandlerFn,
                            [&](fastgltf::sources::Array& vector) {
                                auto slice =
                                    std::span<std::byte>(vector.bytes)
                                        .subspan(bufferView.byteOffset, bufferView.byteLength);

                                BuildTextureFromMemory(slice, outputPath);
                            },
                        },
                        buffer.data);
                },
            },
            image.data);
    }
};
} // namespace sj::build
