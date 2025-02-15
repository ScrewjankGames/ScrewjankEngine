#include "ScrewjankEngine/platform/Vulkan/VulkanHelpers.hpp"
#include <ScrewjankShared/utils/PlatformDetection.hpp>
#if defined(SJ_PLATFORM_WINDOWS) || defined(SJ_PLATFORM_LINUX)

// Library Headers
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>

// Engine Headers
#include <ScrewjankEngine/rendering/Renderer.hpp>
#include <ScrewjankEngine/platform/Windows/GLFW_Window.hpp>
#include <ScrewjankEngine/system/memory/MemSpace.hpp>

namespace sj {
    Window* Window::GetInstance()
    {
        static Window window;
        return &window;
    }

    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        const char* actionStr = (action == GLFW_PRESS)     ? "Pressed"
                                : (action == GLFW_RELEASE) ? "Released"
                                                           : "Held";

        SJ_ENGINE_LOG_INFO("Key {}", actionStr);
    }

    void Window::Init()
    {
        SJ_ENGINE_LOG_INFO("Creating Windows window");
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_NativeWindow = glfwCreateWindow(1280, 720, "Vulkan window", nullptr, nullptr);

        glfwSetKeyCallback(m_NativeWindow, KeyCallback);

        ImGui_ImplGlfw_InitForVulkan(m_NativeWindow, true);
    }

    void Window::DeInit()
    {
        SJ_ENGINE_LOG_INFO("Terminating Windows window");
        glfwDestroyWindow(m_NativeWindow);
        glfwTerminate();
    }

    void Window::ProcessEvents()
    {
        // Update
        glfwPollEvents();

        ImGui_ImplGlfw_NewFrame();
        return;
    }

    bool Window::IsWindowClosed() const
    {
        return glfwWindowShouldClose(m_NativeWindow);
    }

    Viewport Window::GetViewportSize() const
    {
        int width;
        int height;

        glfwGetFramebufferSize(m_NativeWindow, &width, &height);
        
        Viewport size = {(uint32_t)width, (uint32_t)height};
        return size;
    }

#ifdef SJ_VULKAN_SUPPORT
    std::span<const char*> Window::GetRequiredVulkanExtenstions() const
    {
        uint32_t extension_count = 0;
        const char** extensions;
        
        MemSpaceScope _(Renderer::WorkBuffer());
        extensions = glfwGetRequiredInstanceExtensions(&extension_count);

        return std::span{extensions, extension_count};
    }

    VkSurfaceKHR Window::CreateWindowSurface(VkInstance instance) const
    {
        VkSurfaceKHR surface;
        VkResult success = glfwCreateWindowSurface(instance, m_NativeWindow, &sj::g_vkAllocationFns, &surface);
        SJ_ASSERT(success == VK_SUCCESS, "Failed to create vulkan window surface");

        return surface;
    }
#endif
} // namespace sj
#endif