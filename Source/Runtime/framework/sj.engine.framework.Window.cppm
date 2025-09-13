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

export module sj.engine.framework.Window;
import sj.std.math;
import sj.engine.rendering.vk.Primitives;

export namespace sj
{

    /**
     * @brief Platform-specific implementation of a windows window
     */
    class Window
    {
    public:
        Window(const char* windowTitle, Vec2 dimensions)
        {
            SJ_ENGINE_LOG_INFO("Creating glfw window");
            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            m_NativeWindow = glfwCreateWindow(dimensions.GetX(),
                                              dimensions.GetY(),
                                              windowTitle,
                                              nullptr,
                                              nullptr);

            glfwSetKeyCallback(m_NativeWindow, KeyCallback);

            ImGui_ImplGlfw_InitForVulkan(m_NativeWindow, true);
        }

        ~Window()
        {
            SJ_ENGINE_LOG_INFO("Terminating glfw window");
            glfwDestroyWindow(m_NativeWindow);
            glfwTerminate();
        }

        /**
         * Pump the window's event queue
         */
        void ProcessEvents()
        {
            // Update
            glfwPollEvents();

            ImGui_ImplGlfw_NewFrame();
            return;
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

        GLFWwindow* m_NativeWindow = nullptr;
    };
} // namespace sj
