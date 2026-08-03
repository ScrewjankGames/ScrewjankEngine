module;

#include <SDL3/SDL_gpu.h>

#include <string_view>

export module sj.engine.rendering.pipelines:GraphicsPipeline;
import sj.engine.rendering.Upload;
import sj.engine.rendering.VertexDescription;
import sj.datadefs.assets.Mesh;

import sj.engine.rendering.resources;
import sj.std;

export namespace sj
{
struct ModelUniformBufferObject
{
    mat44 modelToWorld;
};

struct DefaultMaterialUniformBufferObject
{
    vec4 baseAlbedoColor = {};
    uint32_t useTexSampler = 0;
};

struct GraphicsPipeline
{
    std::string_view vertexShaderPath;
    std::string_view fragmentShaderPath;

    u32 numVertexUniformBuffers = 0;
    u32 numFragSamplers = 0;
    u32 numFragUniformBuffers = 0;

    SDL_GPUVertexInputState vertexState;
    SDL_GPUPrimitiveType primitiveType = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    SDL_GPURasterizerState rasterizerState;
    SDL_GPUMultisampleState multisampleState;
    SDL_GPUDepthStencilState depthState;
};

[[nodiscard]]
SDL_GPUGraphicsPipeline* MakeGraphicsPipeline(SDL_GPUDevice* device,
                                              const GraphicsPipeline& p,
                                              TextureResource& drawTarget,
                                              TextureResource* depthTarget)
{
    SDL_GPUShader* vertexShader = UploadShader(device,
                                               p.vertexShaderPath,
                                               SDL_GPUShaderCreateInfo {
                                                   .entrypoint = "main",
                                                   .format = SDL_GPU_SHADERFORMAT_SPIRV,
                                                   .stage = SDL_GPU_SHADERSTAGE_VERTEX,
                                                   .num_uniform_buffers = p.numVertexUniformBuffers,
                                               });

    SDL_GPUShader* fragmentShader = UploadShader(device,
                                                 p.fragmentShaderPath,
                                                 SDL_GPUShaderCreateInfo {
                                                     .entrypoint = "main",
                                                     .format = SDL_GPU_SHADERFORMAT_SPIRV,
                                                     .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
                                                     .num_samplers = p.numFragSamplers,
                                                     .num_uniform_buffers = p.numFragUniformBuffers,
                                                 });

    std::array colorTargets {
        SDL_GPUColorTargetDescription {.format = drawTarget.GetFormat()},
    };

    SDL_GPUGraphicsPipelineCreateInfo info {
        .vertex_shader = vertexShader,
        .fragment_shader = fragmentShader,
        .vertex_input_state = p.vertexState,
        .primitive_type = p.primitiveType,
        .rasterizer_state = p.rasterizerState,
        .multisample_state = p.multisampleState,
        .depth_stencil_state = p.depthState,
        .target_info = SDL_GPUGraphicsPipelineTargetInfo {
            .color_target_descriptions = colorTargets.data(),
            .num_color_targets = colorTargets.size(),
            .depth_stencil_format =
                depthTarget ? depthTarget->GetFormat() : SDL_GPUTextureFormat {},
            .has_depth_stencil_target = depthTarget != nullptr,
        }};

    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device, &info);
    SDL_ReleaseGPUShader(device, vertexShader);
    SDL_ReleaseGPUShader(device, fragmentShader);

    return pipeline;
}

const inline GraphicsPipeline gDefaultGraphicsPipeline {
    .vertexShaderPath = "Data/Engine/Shaders/Default.vert.spv",
    .fragmentShaderPath = "Data/Engine/Shaders/Default.frag.spv",
    .numVertexUniformBuffers = 2,
    .numFragSamplers = 1,
    .numFragUniformBuffers = 1,
    .vertexState = VertexInputState<MeshVertex>,
    .rasterizerState =
        SDL_GPURasterizerState {
            .fill_mode = SDL_GPUFillMode::SDL_GPU_FILLMODE_FILL,
            .cull_mode = SDL_GPUCullMode::SDL_GPU_CULLMODE_BACK,
            .front_face = SDL_GPUFrontFace::SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
            .enable_depth_clip = true,
        },
    .multisampleState =
        SDL_GPUMultisampleState {
            .sample_count = SDL_GPUSampleCount::SDL_GPU_SAMPLECOUNT_1,
            .sample_mask = 0,
            .enable_mask = false,
            .enable_alpha_to_coverage = false,
        },
    .depthState =
        SDL_GPUDepthStencilState {
            .compare_op = SDL_GPU_COMPAREOP_GREATER,
            .enable_depth_test = true,
            .enable_depth_write = true,
        },
};

#ifndef SJ_GOLD
struct DebugVertex
{
    vec3 pos = {};
    vec3 color = {};
};

const inline GraphicsPipeline gDebugGraphicsPipeline {
    .vertexShaderPath = "Data/Engine/Shaders/Debug.vert.spv",
    .fragmentShaderPath = "Data/Engine/Shaders/Debug.frag.spv",
    .numVertexUniformBuffers = 1,
    .numFragSamplers = 0,
    .numFragUniformBuffers = 0,
    .vertexState = VertexInputState<DebugVertex>,
    .primitiveType = SDL_GPU_PRIMITIVETYPE_LINELIST,
    .rasterizerState =
        SDL_GPURasterizerState {
            .fill_mode = SDL_GPUFillMode::SDL_GPU_FILLMODE_LINE,
            .cull_mode = SDL_GPUCullMode::SDL_GPU_CULLMODE_NONE,
            .front_face = SDL_GPUFrontFace::SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
            .enable_depth_clip = false,
        },
    .multisampleState =
        SDL_GPUMultisampleState {
            .sample_count = SDL_GPUSampleCount::SDL_GPU_SAMPLECOUNT_1,
            .sample_mask = 0,
            .enable_mask = false,
            .enable_alpha_to_coverage = false,
        },
    .depthState =
        SDL_GPUDepthStencilState {
            .compare_op = SDL_GPU_COMPAREOP_GREATER,
            .enable_depth_test = false,
            .enable_depth_write = false,
        },
};
#endif
} // namespace sj