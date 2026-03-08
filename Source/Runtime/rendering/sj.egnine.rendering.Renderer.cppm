module;

// Screwjank Headers
#include <ScrewjankStd/PlatformDetection.hpp>
#include <ScrewjankStd/Log.hpp>
#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankDataDefinitions/Assets/AssetType.hpp>

// Library Headers
#include <SDL3/SDL_gpu.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

// STD Headers
#include <cstddef>
#include <concepts>
#include <fstream>
#include <functional>
#include <filesystem>
#include <type_traits>

export module sj.engine.rendering.Renderer;
import sj.engine.rendering.Events;
import sj.engine.rendering.TextureResource;
import sj.engine.rendering.SamplerResource;

import sj.engine.core.Program;
import sj.engine.core.Window;

import sj.engine.system.threading.ThreadContext;
import sj.engine.system.memory.MemorySystem;

import sj.datadefs.assets.Texture;
import sj.datadefs.assets.Mesh;

import sj.std;

export namespace sj
{

struct MeshBuffer
{
    SDL_GPUBuffer* buffer = nullptr;
    uint32_t indexBufferOffset = 0;
    uint32_t numIndices = 0;

    SDL_GPUBufferBinding GetVertexBinding()
    {
        return SDL_GPUBufferBinding {.buffer = buffer};
    }

    SDL_GPUBufferBinding GetIndexBinding()
    {
        return SDL_GPUBufferBinding {.buffer = buffer, .offset = indexBufferOffset};
    }
};

class Renderer
{
public:
    static free_list_allocator* WorkBuffer()
    {
        static free_list_allocator g_workBufferResource;
        return &g_workBufferResource;
    }

    Renderer() = default;

    void Initialize(auto& program)
    {
        mPresentCallbackFn = [&program](const PresentEvent& evt) -> void {
            program.template EmitEvent<const PresentEvent&>(evt);
        };

        mDisplay = program.template GetModule<Window>();

        free_list_allocator* workBuffer = WorkBuffer();
        workBuffer->init(4_MiB, *MemorySystem::GetRootMemoryResource());
        MemorySystem::TrackMemoryResource(workBuffer);

        mDevice = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, g_IsDebugBuild, "vulkan");
        SJ_ASSERT(mDevice, "Failed to acquire GPU");
        SDL_ClaimWindowForGPUDevice(mDevice, mDisplay->GetWindowHandle());

        InitRenderTargets();
        InitDefaultPipeline();

        mDummyMeshBuffer = UploadMesh("Data/Engine/viking_room.sj_mesh");
        mDummySampler = UploadSamplerTexture("Data/Engine/viking_room.sj_tex");
    }

    ~Renderer()
    {
        SDL_ReleaseGPUGraphicsPipeline(mDevice, mDefaultGraphicsPipeline);
        mDummySampler.Release();
        SDL_ReleaseGPUBuffer(mDevice, mDummyMeshBuffer.buffer);
        mDepthTarget.Release();
        mDrawTarget.Release();
        SDL_ReleaseWindowFromGPUDevice(mDevice, mDisplay->GetWindowHandle());
        SDL_DestroyGPUDevice(mDevice);
    }

    // By default, renderer will handle call to present by writing to swapchain.
    // Editor may intercept and put the image into a panel
    bool ProcessEvent(const PresentEvent& evt)
    {
        SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(mDevice);

        SDL_GPUTexture* swapchainTexture = nullptr;
        uint32_t swapchainWidth = 0;
        uint32_t swapchainHeight = 0;
        SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer,
                                              mDisplay->GetWindowHandle(),
                                              &swapchainTexture,
                                              &swapchainWidth,
                                              &swapchainHeight);

        SDL_GPUBlitInfo blitInfo {
            .source = SDL_GPUBlitRegion {.texture = evt.image, .w = evt.width, .h = evt.height},
            .destination = SDL_GPUBlitRegion {.texture = swapchainTexture,
                                              .w = swapchainWidth,
                                              .h = swapchainHeight},
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .clear_color {0.0f, 0.0f, 0.0f, 1.0f},
            .filter = SDL_GPU_FILTER_NEAREST};
        SDL_BlitGPUTexture(commandBuffer, &blitInfo);
        SDL_SubmitGPUCommandBuffer(commandBuffer);

        return true;
    }

    void NewFrame()
    {
        if(!mImGuiEnabled)
            return;

        // Start the Dear ImGui frame
        ImGui_ImplSDLGPU3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
    }

    void Process(float _)
    {
    }

    void EndFrame()
    {
    }

    void InitImGuiBackend()
    {
        ImGui_ImplSDLGPU3_InitInfo info {
            .Device = mDevice,
            .ColorTargetFormat =
                SDL_GetGPUSwapchainTextureFormat(mDevice, mDisplay->GetWindowHandle())};

        ImGui_ImplSDL3_InitForSDLGPU(mDisplay->GetWindowHandle());
        ImGui_ImplSDLGPU3_Init(&info);

        mImGuiEnabled = true;
    }

    void TeardownImGuiBackend()
    {
        ImGui_ImplSDL3_Shutdown();
        ImGui_ImplSDLGPU3_Shutdown();
    }

    void Render(const Mat44& cameraMatrix)
    {
        SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(mDevice);

        uint32_t displayWidth = static_cast<uint32_t>(mDisplay->GetViewportSize().GetX());
        uint32_t displayHeight = static_cast<uint32_t>(mDisplay->GetViewportSize().GetY());

        if(displayWidth != mDepthTarget.GetWidth() || displayHeight != mDepthTarget.GetHeight())
        {
            mDepthTarget.Resize(displayWidth, displayHeight);
            mDrawTarget.Resize(displayWidth, displayHeight);
        }

        const float aspectRatio =
            static_cast<float>(displayWidth) / static_cast<float>(displayHeight);

        GlobalUniformBufferObject tmpGUBO {
            .model = Mat44(kIdentityTag),
            .view = cameraMatrix.AffineInverse(),
            .projection = PerspectiveProjection(ToRadians(45.0f), aspectRatio, 10000.0f, 0.1f)};

        SDL_PushGPUVertexUniformData(commandBuffer, 0, &tmpGUBO, sizeof(GlobalUniformBufferObject));

        SDL_GPUColorTargetInfo colorTargetInfo {
            .texture = mDrawTarget.Get(),
            .clear_color = {.r = 50 / 255.0f,
                            .g = 50 / 255.0f,
                            .b = 240 / 255.0f,
                            .a = 255 / 255.0f},
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
        };
        SDL_GPUDepthStencilTargetInfo depthTargetInfo {
            .texture = mDepthTarget.Get(),
            .clear_depth = 0.0f,
            .load_op = SDL_GPULoadOp::SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPUStoreOp::SDL_GPU_STOREOP_DONT_CARE};
        SDL_GPURenderPass* renderPass =
            SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, &depthTargetInfo);

        SDL_BindGPUGraphicsPipeline(renderPass, mDefaultGraphicsPipeline);

        SDL_GPUBufferBinding vertexBinding = mDummyMeshBuffer.GetVertexBinding();
        SDL_GPUBufferBinding indexBinding = mDummyMeshBuffer.GetIndexBinding();
        SDL_GPUTextureSamplerBinding samplerBinding {.texture = mDummySampler.GetTexture(),
                                                     .sampler = mDummySampler.GetSampler()};

        SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBinding, 1);
        SDL_BindGPUIndexBuffer(renderPass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_32BIT);
        SDL_BindGPUFragmentSamplers(renderPass, 0, &samplerBinding, 1);
        SDL_DrawGPUIndexedPrimitives(renderPass, mDummyMeshBuffer.numIndices, 1, 0, 0, 0);
        SDL_EndGPURenderPass(renderPass);

        SDL_SubmitGPUCommandBuffer(commandBuffer);

        mPresentCallbackFn(PresentEvent {.image = mDrawTarget.Get(),
                                         .width = mDrawTarget.GetWidth(),
                                         .height = mDrawTarget.GetHeight()});
    }

    void RenderImGui(ImDrawData* drawData)
    {
        SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(mDevice);
        ImGui_ImplSDLGPU3_PrepareDrawData(drawData, commandBuffer);

        SDL_GPUTexture* swapchainTexture = nullptr;
        uint32_t renderTargetWidth = 0;
        uint32_t renderTargetHeight = 0;
        SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer,
                                              mDisplay->GetWindowHandle(),
                                              &swapchainTexture,
                                              &renderTargetWidth,
                                              &renderTargetHeight);

        SDL_GPUColorTargetInfo colorTargetInfo {.texture = swapchainTexture,
                                                .load_op = SDL_GPU_LOADOP_LOAD,
                                                .store_op = SDL_GPU_STOREOP_STORE};
        SDL_GPURenderPass* imguiPass =
            SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, nullptr);
        ImGui_ImplSDLGPU3_RenderDrawData(drawData, commandBuffer, imguiPass);
        SDL_EndGPURenderPass(imguiPass);

        SDL_SubmitGPUCommandBuffer(commandBuffer);
    }

    [[nodiscard]]
    SDL_GPUShader* UploadShader(const char* path, SDL_GPUShaderCreateInfo info)
    {
        scratchpad_scope scope = ThreadContext::GetScratchpad();

        info.code_size = std::filesystem::file_size(path);
        dynamic_array<char> code(info.code_size, &scope.get_allocator());

        std::ifstream shaderFile(path, std::ios::binary);
        shaderFile.read(code.data(), info.code_size);

        info.code = reinterpret_cast<uint8_t*>(code.data());
        return SDL_CreateGPUShader(mDevice, &info);
    }

    template <class Fn>
        requires std::invocable<Fn, SDL_GPUCommandBuffer*>
    void ImmediateCommand(Fn&& f)
    {
        SDL_GPUCommandBuffer* immediateBuffer = SDL_AcquireGPUCommandBuffer(mDevice);
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
    SDL_GPUTransferBuffer* UploadToGPU(size_t bufferSizeBytes, Fn&& uploadFn)
    {
        SDL_GPUTransferBufferCreateInfo tbInfo {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                                                .size = static_cast<Uint32>(bufferSizeBytes)};
        SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(mDevice, &tbInfo);

        void* uploadPtr = SDL_MapGPUTransferBuffer(mDevice, transferBuffer, false);
        std::invoke(std::forward<Fn>(uploadFn),
                    std::span(reinterpret_cast<std::byte*>(uploadPtr), bufferSizeBytes));
        SDL_UnmapGPUTransferBuffer(mDevice, transferBuffer);

        return transferBuffer;
    }

    [[nodiscard]]
    MeshBuffer UploadMesh(const char* path)
    {
        std::ifstream file(path, std::ios::binary);
        MeshHeader header = {};
        file.read(reinterpret_cast<char*>(&header), sizeof(MeshHeader));
        SJ_ASSERT(header.type == AssetType::kMesh, "Invalid texture load");

        const uint32_t vertexBufferSize = (sizeof(MeshVertex) * header.numVerts);
        const uint32_t indexBufferSize = (header.indexSize * header.numIndices);
        const uint32_t vertexAndIndexBufferSizeBytes = vertexBufferSize + indexBufferSize;

        SDL_GPUBufferCreateInfo info {.usage =
                                          SDL_GPU_BUFFERUSAGE_VERTEX | SDL_GPU_BUFFERUSAGE_INDEX,
                                      .size = vertexAndIndexBufferSizeBytes};

        MeshBuffer meshBuffer {.buffer = SDL_CreateGPUBuffer(mDevice, &info),
                               .indexBufferOffset = vertexBufferSize,
                               .numIndices = header.numIndices};

        SDL_GPUTransferBuffer* transferBuffer =
            UploadToGPU(vertexAndIndexBufferSizeBytes, [&](std::span<std::byte> uploadBuffer) {
                char* vertexBufferStart = reinterpret_cast<char*>(uploadBuffer.data());
                char* indexBufferStart = vertexBufferStart + vertexBufferSize;

                file.read(vertexBufferStart, vertexBufferSize);
                file.read(indexBufferStart, indexBufferSize);
            });

        ImmediateCommand([&](SDL_GPUCommandBuffer* cmd) {
            CopyPass(cmd, [&](SDL_GPUCopyPass* copyPass) {
                SDL_GPUTransferBufferLocation vertexBufferSrc {.transfer_buffer = transferBuffer,
                                                               .offset = 0};
                SDL_GPUTransferBufferLocation indexBufferSrc {.transfer_buffer = transferBuffer,
                                                              .offset = vertexBufferSize};

                SDL_GPUBufferRegion vertexBufferDest {.buffer = meshBuffer.buffer,
                                                      .offset = 0,
                                                      .size = vertexBufferSize};

                SDL_GPUBufferRegion indexBufferDest {.buffer = meshBuffer.buffer,
                                                     .offset = vertexBufferSize,
                                                     .size = indexBufferSize};

                SDL_UploadToGPUBuffer(copyPass, &vertexBufferSrc, &vertexBufferDest, false);
                SDL_UploadToGPUBuffer(copyPass, &indexBufferSrc, &indexBufferDest, false);
            });
        });

        SDL_ReleaseGPUTransferBuffer(mDevice, transferBuffer);

        return meshBuffer;
    }

    [[nodiscard]]
    SamplerResource UploadSamplerTexture(const char* path)
    {
        std::ifstream file(path, std::ios::binary);
        TextureHeader textureHeader = {};
        file.read(reinterpret_cast<char*>(&textureHeader), sizeof(TextureHeader));
        SJ_ASSERT(textureHeader.asset_type == AssetType::kTexture, "Invalid texture load");

        SamplerResource res = SamplerResource(mDevice,
                                              textureHeader.width,
                                              textureHeader.height,
                                              gDefaultSamplerCreateInfo);

        const size_t textureBufferSizeBytes =
            textureHeader.height * textureHeader.width * textureHeader.bytesPerPixel;

        SDL_GPUTransferBuffer* transferBuffer =
            UploadToGPU(textureBufferSizeBytes, [&](std::span<std::byte> uploadBuffer) {
                file.read(reinterpret_cast<char*>(uploadBuffer.data()), textureBufferSizeBytes);
            });

        ImmediateCommand([&](SDL_GPUCommandBuffer* cmd) {
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

        SDL_ReleaseGPUTransferBuffer(mDevice, transferBuffer);

        return res;
    }

    struct GlobalUniformBufferObject
    {
        Mat44 model;
        Mat44 view;
        Mat44 projection;
    };

private:
    void InitRenderTargets()
    {
        SDL_GPUTextureCreateInfo targetInfo {
            .type = SDL_GPUTextureType::SDL_GPU_TEXTURETYPE_2D,
            .width = static_cast<uint32_t>(mDisplay->GetViewportSize().GetX()),
            .height = static_cast<uint32_t>(mDisplay->GetViewportSize().GetY()),
            .layer_count_or_depth = 1,
            .num_levels = 1};

        targetInfo.format = SDL_GPUTextureFormat::SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        targetInfo.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
        mDrawTarget = TextureResource(mDevice, targetInfo);

        targetInfo.format = SDL_GPUTextureFormat::SDL_GPU_TEXTUREFORMAT_D16_UNORM;
        targetInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
        mDepthTarget = TextureResource(mDevice, targetInfo);
    }

    void InitDefaultPipeline()
    {
        SDL_GPUShader* vertexShader =
            UploadShader("Data/Engine/Shaders/Default.vert.spv",
                         SDL_GPUShaderCreateInfo {.entrypoint = "main",
                                                  .format = SDL_GPU_SHADERFORMAT_SPIRV,
                                                  .stage = SDL_GPU_SHADERSTAGE_VERTEX,
                                                  .num_uniform_buffers = 1});
        SDL_GPUShader* fragmentShader =
            UploadShader("Data/Engine/Shaders/Default.frag.spv",
                         SDL_GPUShaderCreateInfo {.entrypoint = "main",
                                                  .format = SDL_GPU_SHADERFORMAT_SPIRV,
                                                  .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
                                                  .num_samplers = 1});

        SDL_GPUVertexBufferDescription vertexDesc {
            .slot = 0,
            .pitch = sizeof(MeshVertex),
            .input_rate = SDL_GPUVertexInputRate::SDL_GPU_VERTEXINPUTRATE_VERTEX,
            .instance_step_rate = 0};

        std::array<SDL_GPUVertexAttribute, 3> vertexAttribs = {
            SDL_GPUVertexAttribute {
                .location = 0,
                .buffer_slot = 0,
                .format = SDL_GPUVertexElementFormat::SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                .offset = offsetof(MeshVertex, pos)},
            SDL_GPUVertexAttribute {
                .location = 1,
                .buffer_slot = 0,
                .format = SDL_GPUVertexElementFormat::SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                .offset = offsetof(MeshVertex, normal)},
            SDL_GPUVertexAttribute {
                .location = 2,
                .buffer_slot = 0,
                .format = SDL_GPUVertexElementFormat::SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                .offset = offsetof(MeshVertex, uv)},
        };

        std::array<SDL_GPUColorTargetDescription, 1> colorTargets {
            SDL_GPUColorTargetDescription {.format = mDrawTarget.GetFormat()}};

        SDL_GPUGraphicsPipelineCreateInfo info {
            .vertex_shader = vertexShader,
            .fragment_shader = fragmentShader,
            .vertex_input_state =
                SDL_GPUVertexInputState {.vertex_buffer_descriptions = &vertexDesc,
                                         .num_vertex_buffers = 1,
                                         .vertex_attributes = vertexAttribs.data(),
                                         .num_vertex_attributes = vertexAttribs.size()},
            .primitive_type = SDL_GPUPrimitiveType::SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
            .rasterizer_state =
                SDL_GPURasterizerState {.fill_mode = SDL_GPUFillMode::SDL_GPU_FILLMODE_FILL,
                                        .cull_mode = SDL_GPUCullMode::SDL_GPU_CULLMODE_BACK,
                                        .front_face =
                                            SDL_GPUFrontFace::SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
                                        .enable_depth_clip = true},
            .multisample_state =
                SDL_GPUMultisampleState {.sample_count = SDL_GPUSampleCount::SDL_GPU_SAMPLECOUNT_1,
                                         .sample_mask = 0,
                                         .enable_mask = false,
                                         .enable_alpha_to_coverage = false},
            .depth_stencil_state =
                SDL_GPUDepthStencilState {.compare_op = SDL_GPU_COMPAREOP_GREATER,
                                          .enable_depth_test = true,
                                          .enable_depth_write = true},
            .target_info = SDL_GPUGraphicsPipelineTargetInfo {
                .color_target_descriptions = colorTargets.data(),
                .num_color_targets = colorTargets.size(),
                .depth_stencil_format = mDepthTarget.GetFormat(),
                .has_depth_stencil_target = true,
            }};

        mDefaultGraphicsPipeline = SDL_CreateGPUGraphicsPipeline(mDevice, &info);
        SDL_ReleaseGPUShader(mDevice, vertexShader);
        SDL_ReleaseGPUShader(mDevice, fragmentShader);
    }

    /**
     * Computes perpsective projection matrix
     * @param verticalFOV: Vertical FOV of the view frustrum
     * @param aspectRatio: Render surface width / height
     * @param near: Near render plane
     * @param far: Far render plane
     *
     * When you're smarter, see:
     * https://www.youtube.com/watch?v=U0_ONQQ5ZNM
     * https://www.youtube.com/watch?v=YO46x8fALzE
     */
    Mat44 PerspectiveProjection(float verticalFOV, float aspectRatio, float near, float far)
    {
        const float invTanHalfvFov = 1.0f / std::tan(verticalFOV / 2.0f);

        Mat44 res;
        res.Set<0, 0>(invTanHalfvFov / aspectRatio);
        res.Set<1, 1>(invTanHalfvFov);
        res.Set<2, 2>(far / (near - far));
        res.Set<2, 3>(-1.0f);
        res.Set<3, 2>((near * far) / (near - far));

        return res;
    }

    std::function<void(const PresentEvent&)> mPresentCallbackFn;

    Window* mDisplay = nullptr;

    bool mImGuiEnabled = false;

    SDL_GPUDevice* mDevice = nullptr;
    TextureResource mDrawTarget {};
    TextureResource mDepthTarget {};

    MeshBuffer mDummyMeshBuffer = {};
    SamplerResource mDummySampler;

    SDL_GPUGraphicsPipeline* mDefaultGraphicsPipeline = nullptr;
};
} // namespace sj