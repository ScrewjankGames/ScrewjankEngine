// STD Headers

// Library Headers
#ifdef SJ_VULKAN_SUPPORT
    #define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// Screwjank Headers
#include <platform/PlatformDetection.hpp>
#include <platform/Windows/WindowsWindow.hpp>

namespace sj {
    WindowsWindow::WindowsWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_NativeWindow = glfwCreateWindow(1280, 720, "Vulkan window", nullptr, nullptr);
    }

    WindowsWindow::~WindowsWindow()
    {
        glfwDestroyWindow(m_NativeWindow);
        glfwTerminate();
    }

    void WindowsWindow::ProcessEvents()
    {
        // Update
        glfwPollEvents();

        return;
    }

    bool WindowsWindow::WindowClosed() const
    {
        return glfwWindowShouldClose(m_NativeWindow);
    }

    Window::FrameBufferSize WindowsWindow::GetFrameBufferSize() const
    {
        int width;
        int height;

        glfwGetFramebufferSize(m_NativeWindow, &width, &height);
        
        FrameBufferSize size = {(uint32_t)width, (uint32_t)height};
        return size;
    }

#ifdef SJ_VULKAN_SUPPORT
    Vector<const char*> WindowsWindow::GetRequiredVulkanExtenstions() const
    {
        uint32_t extension_count = 0;
        const char** extensions;
        extensions = glfwGetRequiredInstanceExtensions(&extension_count);

        Vector<const char*> extensions_vector;
        extensions_vector.Reserve(extension_count);

        for (size_t i = 0; i < extension_count; i++)
        {
            extensions_vector.PushBack(extensions[i]);
        }

        return extensions_vector;
    }

    VkSurfaceKHR WindowsWindow::CreateWindowSurface(VkInstance instance) const
    {
        VkSurfaceKHR surface;
        VkResult success = glfwCreateWindowSurface(instance, m_NativeWindow, nullptr, &surface);
        SJ_ASSERT(success == VK_SUCCESS, "Failed to create vulkan window surface");

        return surface;
    }
#endif
} // namespace sj
