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
import sj.engine.rendering.vk.Primitives;

export namespace sj
{

/**
 * @brief Platform-specific implementation of a windows window
 */
class Window : public IModule
{
public:
    Window(Program& program, const char* windowTitle, Vec2 dimensions)
    {
        SJ_ENGINE_LOG_INFO("Creating window");
        m_NativeWindow = SDL_CreateWindow(windowTitle,
                                          static_cast<int>(dimensions.GetX()),
                                          static_cast<int>(dimensions.GetY()),
                                          SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

        SJ_ASSERT(m_NativeWindow != nullptr, "Failed to create SDL window");
    }

    ~Window()
    {
        SJ_ENGINE_LOG_INFO("Terminating window");
        SDL_DestroyWindow(m_NativeWindow);
    }

    /**
     * @return Window size in pixels
     */
    [[nodiscard]] Vec2 GetViewportSize() const
    {
        int width = 0;
        int height = 0;

        SDL_GetWindowSize(m_NativeWindow, &width, &height);

        return Vec2(static_cast<float>(width), static_cast<float>(height));
    }

    SDL_Window* GetWindowHandle()
    {
        return m_NativeWindow;
    }

#ifdef SJ_VULKAN_SUPPORT
    /**
     * @return Extensions Vulkan API must support to support this window.
     */
    [[nodiscard]] std::span<const char* const> GetRequiredVulkanExtenstions() const
    {
        uint32_t extension_count = 0;
        const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&extension_count);
        return std::span {extensions, extension_count};
    }

    /**
     * @return The Vulkan presentation surface for this window
     */
    VkSurfaceKHR CreateWindowSurface(VkInstance instance) const
    {
        VkSurfaceKHR surface {};
        [[maybe_unused]] bool success =
            SDL_Vulkan_CreateSurface(m_NativeWindow, instance, sj::g_vkAllocationFns, &surface);
    #ifndef SJ_GOLD
        if(!success)
        {
            const char* error = SDL_GetError();
            SJ_ASSERT(false, "Failed to create vulkan window surface. Error: {}", error);
        }
    #endif
        return surface;
    }
#endif

private:
    SDL_Window* m_NativeWindow = nullptr;
};
} // namespace sj
