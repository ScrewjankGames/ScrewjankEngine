#include <ScrewjankStd/PlatformDetection.hpp>
#if defined(SJ_PLATFORM_WINDOWS) || defined(SJ_PLATFORM_LINUX)

// Library Headers
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>

// Engine Headers
#include <ScrewjankEngine/platform/Windows/GLFW_Window.hpp>
#include <ScrewjankStd/Log.hpp>
#include <ScrewjankStd/Assert.hpp>

import sj.engine.framework.Engine;
import sj.engine.system.memory.MemorySystem;
import sj.engine.rendering.Renderer;
import sj.engine.rendering.vk;

namespace sj
{
    Window* Window::GetInstance()
    {
        static Window window;
        return &window;
    }

    static void KeyCallback([[maybe_unused]] GLFWwindow* window,
                            [[maybe_unused]] int key,
                            [[maybe_unused]] int scancode,
                            [[maybe_unused]] int action,
                            [[maybe_unused]] int mods)
    {
        const char* actionStr = (action == GLFW_PRESS)     ? "Pressed"
                                : (action == GLFW_RELEASE) ? "Released"
                                                           : "Held";

        SJ_ENGINE_LOG_INFO("Key {}", actionStr);
    }

    void Window::Init()
    {
        SJ_ENGINE_LOG_INFO("Creating glfw window");
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_NativeWindow = glfwCreateWindow(1280, 720, Engine::GetGameName().data(), nullptr, nullptr);

        glfwSetKeyCallback(m_NativeWindow, KeyCallback);

        ImGui_ImplGlfw_InitForVulkan(m_NativeWindow, true);
    }

    void Window::DeInit()
    {
        SJ_ENGINE_LOG_INFO("Terminating glfw window");
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
        int width = 0;
        int height = 0;

        glfwGetFramebufferSize(m_NativeWindow, &width, &height);

        Viewport size = {.Width=(uint32_t)width, .Height=(uint32_t)height};
        return size;
    }

    #ifdef SJ_VULKAN_SUPPORT
    std::span<const char*> Window::GetRequiredVulkanExtenstions() const
    {
        uint32_t extension_count = 0;
        const char** extensions = nullptr;

        extensions = glfwGetRequiredInstanceExtensions(&extension_count);

        return std::span {extensions, extension_count};
    }

    VkSurfaceKHR Window::CreateWindowSurface(VkInstance instance) const
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
} // namespace sj
#endif