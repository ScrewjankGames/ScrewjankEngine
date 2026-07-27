module;

#include <SDL3/SDL_gpu.h>

#include <string_view>

export module sj.engine.rendering.materials:DefaultMaterial;
import sj.std.math;
import sj.datadefs.AssetDB;
import sj.datadefs.assets.Mesh;
import sj.engine.rendering.Pipeline;
import sj.engine.rendering.VertexDescription;

export namespace sj
{

struct DefaultMaterial
{
    vec4 baseAlbedo {1.0f, 1.0f, 1.0f, 1.0f};
    AssetID albedoTexture = kInvalidAssetID;
};

const inline Pipeline gDefaultMaterialPipeline {
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
} // namespace sj