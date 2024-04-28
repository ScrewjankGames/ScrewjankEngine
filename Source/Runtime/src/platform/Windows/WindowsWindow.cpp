#include <ScrewjankEngine/platform/PlatformDetection.hpp>

#ifdef SJ_PLATFORM_WINDOWS
// STD Headers

// Library Headers
#ifdef SJ_VULKAN_SUPPORT
    #define GLFW_INCLUDE_VULKAN
#endif

#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>

// Screwjank Headers
#include <ScrewjankEngine/platform/Windows/WindowsWindow.hpp>

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
    dynamic_vector<const char*> Window::GetRequiredVulkanExtenstions() const
    {
        uint32_t extension_count = 0;
        const char** extensions;
        extensions = glfwGetRequiredInstanceExtensions(&extension_count);

        dynamic_vector<const char*> extensions_vector;
        extensions_vector.reserve(extension_count);

        for (size_t i = 0; i < extension_count; i++)
        {
            extensions_vector.push_back(extensions[i]);
        }

        return extensions_vector;
    }

    VkSurfaceKHR Window::CreateWindowSurface(VkInstance instance) const
    {
        VkSurfaceKHR surface;
        VkResult success = glfwCreateWindowSurface(instance, m_NativeWindow, nullptr, &surface);
        SJ_ASSERT(success == VK_SUCCESS, "Failed to create vulkan window surface");

        return surface;
    }
#endif
} // namespace sj
#endif