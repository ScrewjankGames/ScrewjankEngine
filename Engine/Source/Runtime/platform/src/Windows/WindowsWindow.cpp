// STD Headers

// Library Headers
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// Screwjank Headers
#include "platform/Windows/WindowsWindow.hpp"

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
} // namespace sj
