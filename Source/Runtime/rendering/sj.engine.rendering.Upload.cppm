module;
#include <ScrewjankStd/Assert.hpp>

#include <SDL3/SDL_gpu.h>

#include <concepts>
#include <filesystem>
#include <fstream>
#include <functional>
#include <span>

export module sj.engine.rendering.Upload;
import sj.engine.rendering.resources;
import sj.engine.system.threading.ThreadContext;
import sj.datadefs;
import sj.std;

export namespace sj
{

template <class Fn>
    requires std::invocable<Fn, SDL_GPUCommandBuffer*>
void ImmediateCommand(SDL_GPUDevice* device, Fn&& f)
{
    SDL_GPUCommandBuffer* immediateBuffer = SDL_AcquireGPUCommandBuffer(device);
    std::invoke(std::forward<Fn>(f), immediateBuffer);
    SDL_SubmitGPUCommandBuffer(immediateBuffer);
}

template <class Fn>
    requires std::invocable<Fn, SDL_GPUCopyPass*>
void CopyPass(SDL_GPUCommandBuffer* cmd, Fn&& f)
{
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);
    std::invoke(std::forward<Fn>(f), copyPass);
    SDL_EndGPUCopyPass(copyPass);
}

template <class Fn>
    requires std::invocable<Fn, std::span<std::byte>>
void UploadToGPU(SDL_GPUDevice* device,
                 SDL_GPUTransferBuffer* transferBuffer,
                 uZ bufferSizeBytes,
                 Fn&& uploadFn,
                 bool cycle)
{
    void* uploadPtr = SDL_MapGPUTransferBuffer(device, transferBuffer, cycle);
    std::invoke(std::forward<Fn>(uploadFn),
                std::span(reinterpret_cast<std::byte*>(uploadPtr), bufferSizeBytes));
    SDL_UnmapGPUTransferBuffer(device, transferBuffer);
}

template <class Fn>
    requires std::invocable<Fn, std::span<std::byte>>
SDL_GPUTransferBuffer* UploadToGPU(SDL_GPUDevice* device, size_t bufferSizeBytes, Fn&& uploadFn)
{
    SDL_GPUTransferBufferCreateInfo tbInfo {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                                            .size = static_cast<Uint32>(bufferSizeBytes)};
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(device, &tbInfo);

    UploadToGPU(device, transferBuffer, bufferSizeBytes, std::forward<Fn>(uploadFn), false);

    return transferBuffer;
}

[[nodiscard]]
SDL_GPUShader*
UploadShader(SDL_GPUDevice* device, std::string_view path_str, SDL_GPUShaderCreateInfo info)
{
    scratchpad_scope scope = ThreadContext::GetScratchpad();

    std::filesystem::path path(path_str);
    info.code_size = std::filesystem::file_size(path);
    dynamic_array<char> code(info.code_size, &scope);

    std::ifstream shaderFile(path, std::ios::binary);
    shaderFile.read(code.data(), info.code_size);

    info.code = reinterpret_cast<uint8_t*>(code.data());
    return SDL_CreateGPUShader(device, &info);
}

[[nodiscard]]
MeshBuffer UploadMesh(SDL_GPUDevice* device, std::string_view path)
{
    std::ifstream file(path.data(), std::ios::binary);
    MeshHeader header = {};
    file.read(reinterpret_cast<char*>(&header), sizeof(MeshHeader));
    SJ_ASSERT(header.type == AssetType::kMesh, "Invalid texture load");

    const uint32_t vertexBufferSize = (sizeof(MeshVertex) * header.numVerts);
    const uint32_t indexBufferSize = (header.indexSize * header.numIndices);
    const uint32_t vertexAndIndexBufferSizeBytes = vertexBufferSize + indexBufferSize;

    SDL_GPUBufferCreateInfo info {
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX | SDL_GPU_BUFFERUSAGE_INDEX,
        .size = vertexAndIndexBufferSizeBytes,
    };

    BufferResource meshBuffer(device, info);

    SDL_GPUTransferBuffer* transferBuffer =
        UploadToGPU(device, vertexAndIndexBufferSizeBytes, [&](std::span<std::byte> uploadBuffer) {
            char* vertexBufferStart = reinterpret_cast<char*>(uploadBuffer.data());
            char* indexBufferStart = vertexBufferStart + vertexBufferSize;

            file.read(vertexBufferStart, vertexBufferSize);
            file.read(indexBufferStart, indexBufferSize);
        });

    ImmediateCommand(device, [&](SDL_GPUCommandBuffer* cmd) {
        CopyPass(cmd, [&](SDL_GPUCopyPass* copyPass) {
            SDL_GPUTransferBufferLocation vertexBufferSrc {.transfer_buffer = transferBuffer,
                                                           .offset = 0};
            SDL_GPUTransferBufferLocation indexBufferSrc {.transfer_buffer = transferBuffer,
                                                          .offset = vertexBufferSize};

            SDL_GPUBufferRegion vertexBufferDest {.buffer = meshBuffer.GetBuffer(),
                                                  .offset = 0,
                                                  .size = vertexBufferSize};

            SDL_GPUBufferRegion indexBufferDest {.buffer = meshBuffer.GetBuffer(),
                                                 .offset = vertexBufferSize,
                                                 .size = indexBufferSize};

            SDL_UploadToGPUBuffer(copyPass, &vertexBufferSrc, &vertexBufferDest, false);
            SDL_UploadToGPUBuffer(copyPass, &indexBufferSrc, &indexBufferDest, false);
        });
    });

    SDL_ReleaseGPUTransferBuffer(device, transferBuffer);

    return MeshBuffer {
        .buffer = std::move(meshBuffer),
        .numIndices = header.numIndices,
        .indexBufferOffset = vertexBufferSize,
    };
}

template <class Fn>
    requires std::invocable<Fn, std::span<std::byte>>
[[nodiscard]] SamplerResource
UploadSamplerTexture(SDL_GPUDevice* device, const TextureHeader& textureHeader, Fn&& uploadFn)
{
    SamplerResource res = SamplerResource(device,
                                          textureHeader.width,
                                          textureHeader.height,
                                          gDefaultSamplerCreateInfo);

    const size_t textureBufferSizeBytes =
        textureHeader.height * textureHeader.width * textureHeader.bytesPerPixel;

    SDL_GPUTransferBuffer* transferBuffer =
        UploadToGPU(device, textureBufferSizeBytes, std::forward<Fn>(uploadFn));

    ImmediateCommand(device, [&](SDL_GPUCommandBuffer* cmd) {
        CopyPass(cmd, [&](SDL_GPUCopyPass* pass) {
            SDL_GPUTextureTransferInfo srcInfo {
                .transfer_buffer = transferBuffer,
                .offset = 0,
                .pixels_per_row = static_cast<Uint32>(textureHeader.width),
                .rows_per_layer = static_cast<Uint32>(textureHeader.height)};

            SDL_GPUTextureRegion dstRegion {.texture = res.GetTexture(),
                                            .mip_level = 0,
                                            .layer = 0,
                                            .x = 0,
                                            .y = 0,
                                            .z = 0,
                                            .w = static_cast<Uint32>(textureHeader.width),
                                            .h = static_cast<Uint32>(textureHeader.height),
                                            .d = 1};

            SDL_UploadToGPUTexture(pass, &srcInfo, &dstRegion, false);
        });
    });

    SDL_ReleaseGPUTransferBuffer(device, transferBuffer);

    return res;
}

[[nodiscard]]
SamplerResource UploadSamplerTexture(SDL_GPUDevice* device, std::string_view path)
{
    std::ifstream file(path.data(), std::ios::binary);
    TextureHeader textureHeader = {};
    file.read(reinterpret_cast<char*>(&textureHeader), sizeof(TextureHeader));
    SJ_ASSERT(textureHeader.asset_type == AssetType::kTexture, "Invalid texture load");

    return UploadSamplerTexture(device, textureHeader, [&](std::span<std::byte> uploadBuffer) {
        file.read(reinterpret_cast<char*>(uploadBuffer.data()), uploadBuffer.size());
    });
}

} // namespace sj