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
#include <concepts>
#include <fstream>
#include <filesystem>
#include <type_traits>

export module sj.engine.rendering.Renderer;
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

class RenderTarget
{
public:
    RenderTarget() = default;

    RenderTarget(SDL_GPUDevice* device, const SDL_GPUTextureCreateInfo& createInfo)
        : mDevice(device), mCreateInfo(createInfo)
    {
        Create();
    }

    RenderTarget(const RenderTarget& other)
    {
        *this = other;
    }

    RenderTarget(RenderTarget&& other) noexcept
    {
        *this = std::move(other);
    }

    RenderTarget& operator=(const RenderTarget& other)
    {
        mDevice = other.mDevice;
        mCreateInfo = other.mCreateInfo;

        Create();

        return *this;
    }

    RenderTarget& operator=(RenderTarget&& other) noexcept
    {
        mDevice = std::exchange(other.mDevice, nullptr);
        mTexture = std::exchange(other.mTexture, nullptr);
        mCreateInfo = other.mCreateInfo;

        return *this;
    }

    operator SDL_GPUTexture*()
    {
        return mTexture;
    }

    [[nodiscard]] SDL_GPUTextureFormat GetFormat() const
    {
        return mCreateInfo.format;
    }

    [[nodiscard]] uint32_t GetWidth() const
    {
        return mCreateInfo.width;
    }

    [[nodiscard]] uint32_t GetHeight() const
    {
        return mCreateInfo.height;
    }

    void Resize(uint32_t width, uint32_t height)
    {
        Destroy();
        mCreateInfo.width = width;
        mCreateInfo.height = height;
        Create();
    }

    void Destroy()
    {
        SDL_ReleaseGPUTexture(mDevice, mTexture);
        mTexture = nullptr;
    }

private:
    void Create()
    {
        mTexture = SDL_CreateGPUTexture(mDevice, &mCreateInfo);
        SJ_ASSERT(mTexture,
                  "Failed to create render target texture. Format {} Usage {}!",
                  (int)mCreateInfo.format,
                  (int)mCreateInfo.usage);
    }

    SDL_GPUTextureCreateInfo mCreateInfo {};
    SDL_GPUDevice* mDevice = nullptr;
    SDL_GPUTexture* mTexture = nullptr;
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
        mDisplay = program.template GetModule<Window>();

        free_list_allocator* workBuffer = WorkBuffer();
        workBuffer->init(4_MiB, *MemorySystem::GetRootMemoryResource());
        MemorySystem::TrackMemoryResource(workBuffer);

        mDevice = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, g_IsDebugBuild, "vulkan");
        SJ_ASSERT(mDevice, "Failed to acquire GPU");
        SDL_ClaimWindowForGPUDevice(mDevice, mDisplay->GetWindowHandle());

        mDrawTarget = RenderTarget(
            mDevice,
            SDL_GPUTextureCreateInfo {
                .type = SDL_GPUTextureType::SDL_GPU_TEXTURETYPE_2D,
                .format = SDL_GPUTextureFormat::SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                .usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER,
                .width = static_cast<uint32_t>(mDisplay->GetViewportSize().GetX()),
                .height = static_cast<uint32_t>(mDisplay->GetViewportSize().GetY()),
                .layer_count_or_depth = 1,
                .num_levels = 1});

        mDepthTarget =
            RenderTarget(mDevice,
                         SDL_GPUTextureCreateInfo {
                             .type = SDL_GPUTextureType::SDL_GPU_TEXTURETYPE_2D,
                             .format = SDL_GPUTextureFormat::SDL_GPU_TEXTUREFORMAT_D16_UNORM,
                             .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
                             .width = static_cast<uint32_t>(mDisplay->GetViewportSize().GetX()),
                             .height = static_cast<uint32_t>(mDisplay->GetViewportSize().GetY()),
                             .layer_count_or_depth = 1,
                             .num_levels = 1});

        mDefaultVertexShader =
            UploadShader("Data/Engine/Shaders/Default.vert.spv",
                         SDL_GPUShaderCreateInfo {.entrypoint = "main",
                                                  .format = SDL_GPU_SHADERFORMAT_SPIRV,
                                                  .stage = SDL_GPU_SHADERSTAGE_VERTEX,
                                                  .num_uniform_buffers = 1});
        mDefaultFragmentShader =
            UploadShader("Data/Engine/Shaders/Default.frag.spv",
                         SDL_GPUShaderCreateInfo {.entrypoint = "main",
                                                  .format = SDL_GPU_SHADERFORMAT_SPIRV,
                                                  .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
                                                  .num_samplers = 1});

        mDummyMeshBuffer = UploadMesh("Data/Engine/viking_room.sj_mesh");
        mDummyTexture = UploadSamplerTexture("Data/Engine/viking_room.sj_tex");

        SDL_GPUSamplerCreateInfo samplerInfo {
            .min_filter = SDL_GPUFilter::SDL_GPU_FILTER_LINEAR,
            .mag_filter = SDL_GPUFilter::SDL_GPU_FILTER_LINEAR,
            .mipmap_mode = SDL_GPUSamplerMipmapMode::SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
            .address_mode_u = SDL_GPUSamplerAddressMode::SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
            .address_mode_v = SDL_GPUSamplerAddressMode::SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
            .address_mode_w = SDL_GPUSamplerAddressMode::SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
            .mip_lod_bias = 0.0f,
            .max_anisotropy = 1.0f,
            .compare_op = SDL_GPU_COMPAREOP_ALWAYS,
            .min_lod = 0.0f,
            .max_lod = 1.0f,
            .enable_anisotropy = false,
            .enable_compare = false};
        mDummyTextureSampler = SDL_CreateGPUSampler(mDevice, &samplerInfo);

        mDefaultGraphicsPipeline = CreateDefaultGraphicsPipeline();
    }

    ~Renderer()
    {
        SDL_ReleaseGPUGraphicsPipeline(mDevice, mDefaultGraphicsPipeline);
        SDL_ReleaseGPUSampler(mDevice, mDummyTextureSampler);
        SDL_ReleaseGPUTexture(mDevice, mDummyTexture);
        SDL_ReleaseGPUBuffer(mDevice, mDummyMeshBuffer.buffer);
        SDL_ReleaseGPUShader(mDevice, mDefaultFragmentShader);
        SDL_ReleaseGPUShader(mDevice, mDefaultVertexShader);
        mDepthTarget.Destroy();
        mDrawTarget.Destroy();
        SDL_ReleaseWindowFromGPUDevice(mDevice, mDisplay->GetWindowHandle());
        SDL_DestroyGPUDevice(mDevice);
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

        SDL_GPUTexture* swapchainTexture = nullptr;
        uint32_t renderTargetWidth = 0;
        uint32_t renderTargetHeight = 0;
        SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer,
                                              mDisplay->GetWindowHandle(),
                                              &swapchainTexture,
                                              &renderTargetWidth,
                                              &renderTargetHeight);

        // end the frame early if a swapchain texture is not available
        if(swapchainTexture == nullptr)
        {
            SDL_SubmitGPUCommandBuffer(commandBuffer);
            return;
        }

        if(renderTargetWidth != mDepthTarget.GetWidth() ||
           renderTargetHeight != mDepthTarget.GetHeight())
        {
            mDepthTarget.Resize(renderTargetWidth, renderTargetHeight);
            mDrawTarget.Resize(renderTargetWidth, renderTargetHeight);
        }

        const float aspectRatio =
            static_cast<float>(renderTargetWidth) / static_cast<float>(renderTargetHeight);
        GlobalUniformBufferObject tmpGUBO {
            .model = Mat44(kIdentityTag),
            .view = cameraMatrix.AffineInverse(),
            .projection = PerspectiveProjection(ToRadians(45.0f), aspectRatio, 10000.0f, 0.1f)};

        SDL_PushGPUVertexUniformData(commandBuffer, 0, &tmpGUBO, sizeof(GlobalUniformBufferObject));

        SDL_GPUColorTargetInfo colorTargetInfo {
            .texture = swapchainTexture,
            .clear_color = {.r = 50 / 255.0f,
                            .g = 50 / 255.0f,
                            .b = 240 / 255.0f,
                            .a = 255 / 255.0f},
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
        };
        SDL_GPUDepthStencilTargetInfo depthTargetInfo {
            .texture = mDepthTarget,
            .clear_depth = 0.0f,
            .load_op = SDL_GPULoadOp::SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPUStoreOp::SDL_GPU_STOREOP_DONT_CARE};
        SDL_GPURenderPass* renderPass =
            SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, &depthTargetInfo);

        SDL_BindGPUGraphicsPipeline(renderPass, mDefaultGraphicsPipeline);

        SDL_GPUBufferBinding vertexBinding = mDummyMeshBuffer.GetVertexBinding();
        SDL_GPUBufferBinding indexBinding = mDummyMeshBuffer.GetIndexBinding();
        SDL_GPUTextureSamplerBinding samplerBinding {.texture = mDummyTexture,
                                                     .sampler = mDummyTextureSampler};

        SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBinding, 1);
        SDL_BindGPUIndexBuffer(renderPass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_32BIT);
        SDL_BindGPUFragmentSamplers(renderPass, 0, &samplerBinding, 1);
        SDL_DrawGPUIndexedPrimitives(renderPass, mDummyMeshBuffer.numIndices, 1, 0, 0, 0);
        SDL_EndGPURenderPass(renderPass);

        SDL_SubmitGPUCommandBuffer(commandBuffer);
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

    template <class MakeDstFn, class StageFn, class UploadFn>
        requires std::invocable<MakeDstFn, SDL_GPUDevice*> &&
                 std::invocable<StageFn, std::span<std::byte>> &&
                 std::invocable<UploadFn,
                                SDL_GPUCopyPass*,
                                SDL_GPUTransferBuffer*,
                                std::invoke_result_t<MakeDstFn, SDL_GPUDevice*>>
    [[nodiscard]] auto UploadToGPU(size_t stagingBufferSize,
                                   MakeDstFn&& makeDstFn,
                                   StageFn&& stageFn,
                                   UploadFn&& uploadFn)
    {
        auto&& destination = std::invoke(std::forward<MakeDstFn>(makeDstFn), mDevice);
        using DestType = std::remove_cv_t<decltype(destination)>;

        SDL_GPUCommandBuffer* immediateBuffer = SDL_AcquireGPUCommandBuffer(mDevice);
        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(immediateBuffer);
        SDL_GPUTransferBufferCreateInfo tbInfo {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                                                .size = static_cast<Uint32>(stagingBufferSize)};
        SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(mDevice, &tbInfo);

        void* uploadPtr = SDL_MapGPUTransferBuffer(mDevice, transferBuffer, false);
        std::invoke(std::forward<StageFn>(stageFn),
                    std::span(reinterpret_cast<std::byte*>(uploadPtr), stagingBufferSize));
        SDL_UnmapGPUTransferBuffer(mDevice, transferBuffer);

        std::invoke(std::forward<UploadFn>(uploadFn),
                    copyPass,
                    transferBuffer,
                    std::forward<DestType>(destination));

        SDL_ReleaseGPUTransferBuffer(mDevice, transferBuffer);
        SDL_EndGPUCopyPass(copyPass);
        SDL_SubmitGPUCommandBuffer(immediateBuffer);

        return destination;
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

        auto makeDstFn = [&](SDL_GPUDevice* device) -> MeshBuffer {
            SDL_GPUBufferCreateInfo info {.usage = SDL_GPU_BUFFERUSAGE_VERTEX |
                                                   SDL_GPU_BUFFERUSAGE_INDEX,
                                          .size = vertexAndIndexBufferSizeBytes};

            return MeshBuffer {.buffer = SDL_CreateGPUBuffer(mDevice, &info),
                               .indexBufferOffset = vertexBufferSize,
                               .numIndices = header.numIndices};
        };

        auto stageFn = [&](std::span<std::byte> uploadBuffer) {
            char* vertexBufferStart = reinterpret_cast<char*>(uploadBuffer.data());
            char* indexBufferStart = vertexBufferStart + vertexBufferSize;

            file.read(vertexBufferStart, vertexBufferSize);
            file.read(indexBufferStart, indexBufferSize);
        };

        auto uploadFn =
            [&](SDL_GPUCopyPass* copyPass, SDL_GPUTransferBuffer* srcBuffer, MeshBuffer dstBuffer) {
                SDL_GPUTransferBufferLocation vertexBufferSrc {.transfer_buffer = srcBuffer,
                                                               .offset = 0};
                SDL_GPUTransferBufferLocation indexBufferSrc {.transfer_buffer = srcBuffer,
                                                              .offset = vertexBufferSize};

                SDL_GPUBufferRegion vertexBufferDest {.buffer = dstBuffer.buffer,
                                                      .offset = 0,
                                                      .size = vertexBufferSize};

                SDL_GPUBufferRegion indexBufferDest {.buffer = dstBuffer.buffer,
                                                     .offset = vertexBufferSize,
                                                     .size = indexBufferSize};

                SDL_UploadToGPUBuffer(copyPass, &vertexBufferSrc, &vertexBufferDest, false);
                SDL_UploadToGPUBuffer(copyPass, &indexBufferSrc, &indexBufferDest, false);
            };

        return UploadToGPU(vertexAndIndexBufferSizeBytes, makeDstFn, stageFn, uploadFn);
    }

    [[nodiscard]]
    SDL_GPUTexture* UploadSamplerTexture(const char* path)
    {
        std::ifstream file(path, std::ios::binary);
        TextureHeader textureHeader = {};
        file.read(reinterpret_cast<char*>(&textureHeader), sizeof(TextureHeader));
        SJ_ASSERT(textureHeader.asset_type == AssetType::kTexture, "Invalid texture load");

        size_t bufferSize =
            textureHeader.height * textureHeader.width * textureHeader.bytesPerPixel;

        auto makeDstFn = [&](SDL_GPUDevice* device) -> SDL_GPUTexture* {
            SDL_GPUTextureCreateInfo info {
                .type = SDL_GPUTextureType::SDL_GPU_TEXTURETYPE_2D,
                .format = SDL_GPUTextureFormat::SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
                .width = static_cast<Uint32>(textureHeader.width),
                .height = static_cast<Uint32>(textureHeader.height),
                .layer_count_or_depth = 1,
                .num_levels = 1};

            return SDL_CreateGPUTexture(device, &info);
        };

        auto stageFn = [&](std::span<std::byte> uploadBuffer) {
            file.read(reinterpret_cast<char*>(uploadBuffer.data()), bufferSize);
        };

        auto uploadFn = [&](SDL_GPUCopyPass* copyPass,
                            SDL_GPUTransferBuffer* srcBuffer,
                            SDL_GPUTexture* dstTexture) {
            SDL_GPUTextureTransferInfo srcInfo {
                .transfer_buffer = srcBuffer,
                .offset = 0,
                .pixels_per_row = static_cast<Uint32>(textureHeader.width),
                .rows_per_layer = static_cast<Uint32>(textureHeader.height)};

            SDL_GPUTextureRegion dstRegion {.texture = dstTexture,
                                            .mip_level = 0,
                                            .layer = 0,
                                            .x = 0,
                                            .y = 0,
                                            .z = 0,
                                            .w = static_cast<Uint32>(textureHeader.width),
                                            .h = static_cast<Uint32>(textureHeader.height),
                                            .d = 1};

            SDL_UploadToGPUTexture(copyPass, &srcInfo, &dstRegion, false);
        };

        return UploadToGPU(bufferSize, makeDstFn, stageFn, uploadFn);
    }

    struct GlobalUniformBufferObject
    {
        Mat44 model;
        Mat44 view;
        Mat44 projection;
    };

private:
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

    SDL_GPUGraphicsPipeline* CreateDefaultGraphicsPipeline()
    {
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
                .offset = 0},
            SDL_GPUVertexAttribute {
                .location = 1,
                .buffer_slot = 0,
                .format = SDL_GPUVertexElementFormat::SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                .offset = sizeof(float) * 3},
            SDL_GPUVertexAttribute {
                .location = 2,
                .buffer_slot = 0,
                .format = SDL_GPUVertexElementFormat::SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                .offset = sizeof(float) * 6},
        };

        std::array<SDL_GPUColorTargetDescription, 1> colorTargets {SDL_GPUColorTargetDescription {
            .format = SDL_GetGPUSwapchainTextureFormat(mDevice, mDisplay->GetWindowHandle())}};

        SDL_GPUGraphicsPipelineCreateInfo info {
            .vertex_shader = mDefaultVertexShader,
            .fragment_shader = mDefaultFragmentShader,
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

        return SDL_CreateGPUGraphicsPipeline(mDevice, &info);
    }

    Window* mDisplay = nullptr;

    bool mImGuiEnabled = false;

    SDL_GPUDevice* mDevice = nullptr;
    RenderTarget mDepthTarget {};
    RenderTarget mDrawTarget {};

    MeshBuffer mDummyMeshBuffer = {};
    SDL_GPUTexture* mDummyTexture = nullptr;
    SDL_GPUSampler* mDummyTextureSampler = nullptr;

    SDL_GPUGraphicsPipeline* mDefaultGraphicsPipeline = nullptr;

    SDL_GPUShader* mDefaultVertexShader = nullptr;
    SDL_GPUShader* mDefaultFragmentShader = nullptr;
};
} // namespace sj