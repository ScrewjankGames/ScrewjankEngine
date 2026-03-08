module;
#include <SDL3/SDL_gpu.h>

export module sj.engine.rendering.Events;

export namespace sj
{
    struct PresentEvent
    {
        SDL_GPUTexture* image;
        uint32_t width;
        uint32_t height;
    };
}