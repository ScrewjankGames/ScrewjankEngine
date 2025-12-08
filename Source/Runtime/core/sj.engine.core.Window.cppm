module;

// Engine Headers
#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/Log.hpp>

// Library Headers
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>

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
    Window(Program& program, const char* windowTitle, Vec2 dimensions) : m_hostProgram(&program)
    {
        static GLFWerrorfun GLFWerrorCallback = [](int error_code, const char* description) {
            SJ_ENGINE_LOG_ERROR("GLFW Error (code {}): {}", error_code, description);
        };

        SJ_ENGINE_LOG_INFO("Creating glfw window");
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwSetErrorCallback(GLFWerrorCallback);
        m_NativeWindow =
            glfwCreateWindow(dimensions.GetX(), dimensions.GetY(), windowTitle, nullptr, nullptr);
    }

    ~Window()
    {
        SJ_ENGINE_LOG_INFO("Terminating glfw window");
        glfwDestroyWindow(m_NativeWindow);
        glfwTerminate();
    }

    void NewFrame() override
    {
        glfwPollEvents();
        ImGui_ImplGlfw_NewFrame();

        if(IsWindowClosed())
            m_hostProgram->Terminate();
    }

    /**
     * @return true If the window has been instructed to close, else false
     */
    [[nodiscard]] bool IsWindowClosed() const
    {
        return glfwWindowShouldClose(m_NativeWindow);
    }

    /**
     * @return Window size in pixels
     */
    [[nodiscard]] Vec2 GetViewportSize() const
    {
        int width = 0;
        int height = 0;

        glfwGetFramebufferSize(m_NativeWindow, &width, &height);

        return Vec2(static_cast<float>(width), static_cast<float>(height));
    }

    GLFWwindow* GetWindowHandle()
    {
        return m_NativeWindow;
    }

#ifdef SJ_VULKAN_SUPPORT
    /**
     * @return Extensions Vulkan API must support to support this window.
     */
    [[nodiscard]] std::span<const char*> GetRequiredVulkanExtenstions() const
    {
        uint32_t extension_count = 0;
        const char** extensions = nullptr;

        extensions = glfwGetRequiredInstanceExtensions(&extension_count);

        return std::span {extensions, extension_count};
    }

    /**
     * @return The Vulkan presentation surface for this window
     */
    VkSurfaceKHR CreateWindowSurface(VkInstance instance) const
    {
        VkSurfaceKHR surface {};
        [[maybe_unused]] VkResult success =
            glfwCreateWindowSurface(instance, m_NativeWindow, sj::g_vkAllocationFns, &surface);
    #ifndef SJ_GOLD
        if(success != VK_SUCCESS)
        {
            const char* error = nullptr;
            glfwGetError(&error);
            SJ_ASSERT(false, "Failed to create vulkan window surface. Error: {}", error);
        }
    #endif
        return surface;
    }
#endif

private:
    Program* m_hostProgram = nullptr;
    GLFWwindow* m_NativeWindow = nullptr;
};
} // namespace sj
