module;

// Screwjank Headers
#include <ScrewjankStd/PlatformDetection.hpp>
#include <ScrewjankStd/Log.hpp>
#include <ScrewjankStd/Assert.hpp>

// Library Headers
#include <SDL3/SDL_gpu.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

// STD Headers
#include <algorithm>
#include <cstddef>
#include <concepts>
#include <fstream>
#include <flat_map>
#include <functional>
#include <filesystem>
#include <type_traits>
#include <string_view>

export module sj.engine.rendering.Renderer;
import sj.engine.Program;
import sj.engine.Window;

import sj.engine.rendering.Events;
import sj.engine.rendering.Upload;
import sj.engine.system.threading.ThreadContext;
import sj.engine.system.memory.MemorySystem;

import sj.engine.rendering.materials;
import sj.engine.rendering.pipelines;
import sj.engine.rendering.resources;

import sj.engine.debug;
import sj.std;
import sj.datadefs;

export namespace sj
{
struct GlobalUniformBufferObject
{
    mat44 view;
    mat44 projection;
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
        mAssetDB = &program.GetAssetDB();

        free_list_allocator* workBuffer = WorkBuffer();
        workBuffer->init(4_MiB, *MemorySystem::GetRootMemoryResource());
        MemorySystem::TrackMemoryResource(workBuffer);

        mDevice = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, g_IsDebugBuild, "vulkan");
        SJ_ASSERT(mDevice, "Failed to acquire GPU");
        SDL_ClaimWindowForGPUDevice(mDevice, mDisplay->GetWindowHandle());

        InitRenderTargets();

        mDefaultGraphicsPipeline =
            MakeGraphicsPipeline(mDevice, gDefaultGraphicsPipeline, mDrawTarget, &mDepthTarget);

#ifndef SJ_GOLD
        InitDebugDraw();
#endif

        // Submit Error Texture
        {
            const TextureHeader errorTexHeader = {.asset_type = AssetType::kTexture,
                                                  .bytesPerPixel = 4,
                                                  .width = 1,
                                                  .height = 1};

            mErrorTextureSampler =
                UploadSamplerTexture(mDevice, errorTexHeader, [](std::span<std::byte> buff) {
                    std::ranges::fill(byte_span_cast<uint32_t>(buff), 0xffff00ff);
                });
        }
    }

    ~Renderer()
    {
        SDL_ReleaseGPUGraphicsPipeline(mDevice, mDefaultGraphicsPipeline);

#ifndef SJ_GOLD
        TeardownDebugDraw();
#endif
        mErrorTextureSampler.Release();
        mMeshes.clear();
        mSamplers.clear();

        mDepthTarget.Release();
        mDrawTarget.Release();
        SDL_ReleaseWindowFromGPUDevice(mDevice, mDisplay->GetWindowHandle());
        SDL_DestroyGPUDevice(mDevice);
    }

    void AddMeshReference(AssetID id)
    {
        auto meshIt = mMeshes.find(id);
        if(meshIt == mMeshes.end())
            meshIt = mMeshes.emplace(id, UploadMesh(mDevice, mAssetDB->GetAssetPath(id))).first;

        meshIt->second.refcount_increment();
    }

    void RemoveMeshReference(AssetID id)
    {
        auto meshIt = mMeshes.find(id);
        SJ_ASSERT(meshIt != mMeshes.end(),
                  "Mesh asset {} is being used after free!",
                  mAssetDB->GetAssetPath(id));

        meshIt->second.refcount_decrement();
    }

    void AddTextureReference(AssetID id)
    {
        if(id == kInvalidAssetID)
            return;

        auto textureIt = mSamplers.find(id);
        if(textureIt != mSamplers.end())
        {
            // Adding reference to already loaded asset
            textureIt->second.refcount_increment();
        }
        else
        {
            // Either uploading, or an error
            std::optional<std::string_view> texturePath = mAssetDB->TryGetAssetPath(id);
            if(texturePath)
            {
                textureIt =
                    mSamplers.emplace(id, UploadSamplerTexture(mDevice, texturePath.value())).first;
                textureIt->second.refcount_increment();
            }
            else
            {
                SJ_ENGINE_LOG_ERROR("Failed to find asset path for asset ID {}", id);
            }
        }
    }

    void RemoveTextureReference(AssetID id)
    {
        if(id == kInvalidAssetID)
            return;

        auto textureIt = mSamplers.find(id);
        if(textureIt != mSamplers.end())
        {
            textureIt->second.refcount_decrement();
        }
        else
        {
            SJ_ENGINE_LOG_ERROR("Reducing refcount for unkown texture ID {}", id);
        }
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
        // Handle window resizes
        uint32_t displayWidth = static_cast<uint32_t>(mDisplay->GetViewportSize().get_x());
        uint32_t displayHeight = static_cast<uint32_t>(mDisplay->GetViewportSize().get_y());

        if(displayWidth != mDepthTarget.GetWidth() || displayHeight != mDepthTarget.GetHeight())
        {
            mDepthTarget.Resize(displayWidth, displayHeight);
            mDrawTarget.Resize(displayWidth, displayHeight);
        }
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

    struct MeshDrawArg
    {
        mat44 modelToWorld = {kIdentityTag};
        AssetID modelId = {};
        AssetID textureId = {};
    };

    void ExecuteMainDrawPass(const mat44& cameraMatrix, std::span<MeshDrawArg> meshes)
    {
        SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(mDevice);

#ifndef SJ_GOLD
        debug::DebugDrawCPUData debugData = debug::GetDebugDrawCPUData();
        {
            const uZ vertsSizeBytes = debugData.vertexBufferSizeBytes;
            const uZ indicesSizeBytes = debugData.indexBufferSizeBytes;

            auto uploadDebugDrawDataFn = [&](std::span<std::byte> mappedTransferBuffer) {
                SJ_ASSERT(mappedTransferBuffer.size() >= vertsSizeBytes + indicesSizeBytes,
                          "Transfer buffer to small");

                std::span vertexBufferDst = mappedTransferBuffer.subspan(0, vertsSizeBytes);
                std::ranges::copy(std::as_bytes(debugData.verts), vertexBufferDst.begin());

                std::span indexBufferDst =
                    mappedTransferBuffer.subspan(vertsSizeBytes, indicesSizeBytes);
                std::ranges::copy(std::as_bytes(debugData.indices), indexBufferDst.begin());
            };

            UploadToGPU(mDevice,
                        mDebugTransferBuffer,
                        debug::kDebugPrimBufferSize,
                        uploadDebugDrawDataFn,
                        true);

            CopyPass(commandBuffer, [&](SDL_GPUCopyPass* copyPass) {
                SDL_GPUTransferBufferLocation vertexBufferSrc {
                    .transfer_buffer = mDebugTransferBuffer,
                    .offset = 0,
                };
                SDL_GPUTransferBufferLocation indexBufferSrc {
                    .transfer_buffer = mDebugTransferBuffer,
                    .offset = static_cast<u32>(vertsSizeBytes),
                };

                SDL_GPUBufferRegion vertexBufferDest {
                    .buffer = mDebugDrawBuffer.GetBuffer(),
                    .offset = 0,
                    .size = static_cast<u32>(vertsSizeBytes),
                };

                SDL_GPUBufferRegion indexBufferDest {
                    .buffer = mDebugDrawBuffer.GetBuffer(),
                    .offset = static_cast<u32>(vertsSizeBytes),
                    .size = static_cast<u32>(indicesSizeBytes),
                };
                SDL_UploadToGPUBuffer(copyPass, &vertexBufferSrc, &vertexBufferDest, true);
                SDL_UploadToGPUBuffer(copyPass, &indexBufferSrc, &indexBufferDest, true);
            });

            debug::ClearDebugDrawCPUBuffers();
        }
#endif

        GlobalUniformBufferObject globalUBO {
            .view = cameraMatrix.affine_inverse(),
            .projection =
                PerspectiveProjection(to_rads(45.0f), mDisplay->GetAspectRatio(), 10000.0f, 0.1f),
        };
        SDL_PushGPUVertexUniformData(commandBuffer,
                                     0,
                                     &globalUBO,
                                     sizeof(GlobalUniformBufferObject));

        SDL_GPUColorTargetInfo colorTargetInfo {
            .texture = mDrawTarget.Get(),
            .clear_color =
                {
                    .r = mClearColor.r,
                    .g = mClearColor.g,
                    .b = mClearColor.b,
                    .a = mClearColor.a,
                },
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
        };
        SDL_GPUDepthStencilTargetInfo depthTargetInfo {
            .texture = mDepthTarget.Get(),
            .clear_depth = 0.0f,
            .load_op = SDL_GPULoadOp::SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPUStoreOp::SDL_GPU_STOREOP_DONT_CARE,
        };
        SDL_GPURenderPass* renderPass =
            SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, &depthTargetInfo);

        SDL_BindGPUGraphicsPipeline(renderPass, mDefaultGraphicsPipeline);
        for(const MeshDrawArg& arg : meshes)
        {
            auto meshIt = mMeshes.find(arg.modelId);
            if(meshIt == mMeshes.end())
            {
                SJ_ENGINE_LOG_ERROR("Failed to draw mesh ID {}", arg.modelId);
                continue;
            }

            ModelUniformBufferObject tmpModelUBO {.modelToWorld = mat44(arg.modelToWorld)};
            SDL_PushGPUVertexUniformData(commandBuffer,
                                         1,
                                         &tmpModelUBO,
                                         sizeof(ModelUniformBufferObject));

            const bool useTexture = arg.textureId != kInvalidAssetID;
            DefaultMaterialUniformBufferObject matUBO {
                .baseAlbedoColor = vec4(1.0f, 1.0f, 1.0f, 1.0f),
                .useTexSampler = useTexture,
            };
            SDL_PushGPUFragmentUniformData(commandBuffer,
                                           0,
                                           &matUBO,
                                           sizeof(DefaultMaterialUniformBufferObject));

            const ref<MeshBuffer>& meshBuffer = meshIt->second;
            SDL_GPUBufferBinding vertexBinding = meshBuffer->GetVertexBinding();
            SDL_GPUBufferBinding indexBinding = meshBuffer->GetIndexBinding();
            SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBinding, 1);
            SDL_BindGPUIndexBuffer(renderPass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

            const SamplerResource& albedoSampler = [&] -> const SamplerResource& {
                if(!useTexture)
                    return mErrorTextureSampler;

                auto textureIt = mSamplers.find(arg.textureId);
                if(textureIt == mSamplers.end())
                    return mErrorTextureSampler;

                return *(textureIt->second);
            }();

            SDL_GPUTextureSamplerBinding samplerBinding {.texture = albedoSampler.GetTexture(),
                                                         .sampler = albedoSampler.GetSampler()};

            SDL_BindGPUFragmentSamplers(renderPass, 0, &samplerBinding, 1);
            SDL_DrawGPUIndexedPrimitives(renderPass, meshBuffer->numIndices, 1, 0, 0, 0);
        }

#ifndef SJ_GOLD
        SDL_BindGPUGraphicsPipeline(renderPass, mDebugGraphicsPipeline);
        for(const auto& prim : debugData.prims)
        {
            const u32 primVerticesOffsetBytes =
                static_cast<u32>(0 + sizeof(DebugVertex) * prim.verticesIndexOffset);
            SDL_GPUBufferBinding vertexBinding {
                .buffer = mDebugDrawBuffer.GetBuffer(),
                .offset = primVerticesOffsetBytes,
            };

            const u32 primIndicesOffsetBytes = static_cast<u32>(
                debugData.vertexBufferSizeBytes + (sizeof(u32) * prim.indicesIndexOffset));
            SDL_GPUBufferBinding indexBinding {
                .buffer = mDebugDrawBuffer.GetBuffer(),
                .offset = primIndicesOffsetBytes,
            };
            SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBinding, 1);
            SDL_BindGPUIndexBuffer(renderPass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_32BIT);
            SDL_DrawGPUIndexedPrimitives(renderPass, prim.numIndices, 1, 0, 0, 0);
        }
#endif

        SDL_EndGPURenderPass(renderPass);

        SDL_SubmitGPUCommandBuffer(commandBuffer);
    }

    void Render()
    {
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

private:
    void InitRenderTargets()
    {
        SDL_GPUTextureCreateInfo targetInfo {
            .type = SDL_GPUTextureType::SDL_GPU_TEXTURETYPE_2D,
            .width = static_cast<uint32_t>(mDisplay->GetViewportSize().get_x()),
            .height = static_cast<uint32_t>(mDisplay->GetViewportSize().get_y()),
            .layer_count_or_depth = 1,
            .num_levels = 1};

        targetInfo.format = SDL_GPUTextureFormat::SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        targetInfo.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
        mDrawTarget = TextureResource(mDevice, targetInfo);

        targetInfo.format = SDL_GPUTextureFormat::SDL_GPU_TEXTUREFORMAT_D16_UNORM;
        targetInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
        mDepthTarget = TextureResource(mDevice, targetInfo);
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
    static mat44 PerspectiveProjection(float verticalFOV, float aspectRatio, float near, float far)
    {
        const float invTanHalfvFov = 1.0f / std::tan(verticalFOV / 2.0f);

        mat44 res;
        res.set<0, 0>(invTanHalfvFov / aspectRatio);
        res.set<1, 1>(invTanHalfvFov);
        res.set<2, 2>(far / (near - far));
        res.set<2, 3>(-1.0f);
        res.set<3, 2>((near * far) / (near - far));

        return res;
    }

#ifndef SJ_GOLD
    void InitDebugDraw()
    {
        debug::InitDebugDraw(MemorySystem::GetDebugMemoryResource());

        mDebugGraphicsPipeline =
            MakeGraphicsPipeline(mDevice, gDebugGraphicsPipeline, mDrawTarget, &mDepthTarget);

        SDL_GPUTransferBufferCreateInfo tbInfo {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = static_cast<u32>(debug::kDebugPrimBufferSize),
        };

        mDebugTransferBuffer = SDL_CreateGPUTransferBuffer(mDevice, &tbInfo);
        SDL_GPUBufferCreateInfo info {
            .usage = SDL_GPU_BUFFERUSAGE_VERTEX | SDL_GPU_BUFFERUSAGE_INDEX,
            .size = static_cast<u32>(debug::kDebugPrimBufferSize),
        };

        mDebugDrawBuffer = BufferResource(mDevice, info);
    }

    void TeardownDebugDraw()
    {
        mDebugDrawBuffer.Release();
        SDL_ReleaseGPUTransferBuffer(mDevice, mDebugTransferBuffer);
        SDL_ReleaseGPUGraphicsPipeline(mDevice, mDebugGraphicsPipeline);
    }
#endif

    color mClearColor = {.r = 50 / 255.0f, .g = 50 / 255.0f, .b = 240 / 255.0f, .a = 255 / 255.0f};

    std::function<void(const PresentEvent&)> mPresentCallbackFn;

    Window* mDisplay = nullptr;
    const AssetDB* mAssetDB = nullptr;

    bool mImGuiEnabled = false;

    SDL_GPUDevice* mDevice = nullptr;
    TextureResource mDrawTarget {};
    TextureResource mDepthTarget {};

    std::flat_map<AssetID, ref<MeshBuffer>> mMeshes;
    std::flat_map<AssetID, ref<SamplerResource>> mSamplers;

    SamplerResource mErrorTextureSampler;

    SDL_GPUGraphicsPipeline* mDefaultGraphicsPipeline = nullptr;

#ifndef SJ_GOLD
    SDL_GPUGraphicsPipeline* mDebugGraphicsPipeline = nullptr;
    SDL_GPUTransferBuffer* mDebugTransferBuffer = nullptr;
    BufferResource mDebugDrawBuffer = {};
#endif
};
} // namespace sj