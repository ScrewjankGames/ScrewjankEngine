module;

#include <SDL3/SDL_gpu.h>

#include <string_view>

export module sj.engine.rendering.Pipeline;
import sj.std.primitives;
import sj.std.math;

export namespace sj
{
struct Pipeline
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

} // namespace sj