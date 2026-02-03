module;

// Engine Headers
#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/Log.hpp>

// Library Headers
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_raii.hpp>

// STD Headers
#include <span>

export module sj.engine.core.Window;
import sj.engine.core.Program;

import sj.std.math;

export namespace sj
{

/**
 * @brief Platform-specific implementation of a windows window
 */
class Window
{
public:
    Window() = default;
    ~Window()
    {
        SDL_DestroyWindow(mWindowHandle);
    };

    void Initialize(auto& program)
    {
        const Config& config = program.GetConfig();
        SJ_ENGINE_LOG_INFO("Creating window");
        mWindowHandle = SDL_CreateWindow(config.program_name.c_str(),
                                          static_cast<int>(config.window_size.GetX()),
                                          static_cast<int>(config.window_size.GetY()),
                                          SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE |
                                              SDL_WINDOW_HIGH_PIXEL_DENSITY);

        SJ_ASSERT(mWindowHandle != nullptr, "Failed to create SDL window");
    }

    void NewFrame()
    {
    }

    void Process(float _)
    {
    }

    void EndFrame()
    {
    }

    /**
     * @return Window size in pixels
     */
    [[nodiscard]] Vec2 GetViewportSize() const
    {
        int width = 0;
        int height = 0;

        SDL_GetWindowSize(mWindowHandle, &width, &height);

        return Vec2(static_cast<float>(width), static_cast<float>(height));
    }

    SDL_Window* GetWindowHandle()
    {
        return mWindowHandle;
    }

private:
    SDL_Window* mWindowHandle = nullptr;
};
} // namespace sj
