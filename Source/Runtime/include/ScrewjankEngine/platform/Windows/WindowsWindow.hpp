#pragma once
#include <ScrewjankShared/utils/PlatformDetection.hpp>

#ifdef SJ_PLATFORM_WINDOWS
// STD Headers

// Library Headers
#ifdef SJ_VULKAN_SUPPORT
    #include <vulkan/vulkan.h>
#endif

// Screwjank Headers
#include <ScrewjankEngine/core/Window.hpp>
#include <ScrewjankEngine/containers/Vector.hpp>

struct GLFWwindow;

namespace sj {

    /**
     * @brief Platform-specific implementation of a windows window
     */
    class Window
    {
      public:
        /**
         * Provides global access to the primary render surface 
         */
        static Window* GetInstance();
        
        void Init();
        void DeInit();

        /**
         * Pump the window's event queue
         */
        void ProcessEvents();

        /**
         * @return true If the window has been instructed to close, else false
         */
        bool IsWindowClosed() const;

        /**
         * @return Window size in pixels
         */
        Viewport GetViewportSize() const;

#ifdef SJ_VULKAN_SUPPORT
        /**
         * @return Extensions Vulkan API must support to support this window.   
         */
        dynamic_vector<const char*> GetRequiredVulkanExtenstions() const;

        /**
         * @return The Vulkan presentation surface for this window
         */
        VkSurfaceKHR CreateWindowSurface(VkInstance instance) const;
#endif

      private:
        Window() = default;
        ~Window() = default;

        GLFWwindow* m_NativeWindow;
    };

} // namespace sj
#endif