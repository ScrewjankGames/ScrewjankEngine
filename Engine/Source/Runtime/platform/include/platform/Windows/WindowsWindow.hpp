#pragma once

// STD Headers

// Library Headers
#ifdef SJ_VULKAN_SUPPORT
    #include <vulkan/vulkan.h>
#endif

// Screwjank Headers
#include "core/Window.hpp"
#include "containers/Vector.hpp"
#include "platform/PlatformDetection.hpp"

struct GLFWwindow;

namespace sj {

    /**
     * @brief Platform-specific implementation of a windows window
     */
    class WindowsWindow : public Window
    {
      public:
        /**
         * Constructor
         */
        WindowsWindow();

        /**
         * Destructor
         */
        ~WindowsWindow();

        /**
         * Pump the window's event queue
         */
        void ProcessEvents() override;

        /**
         * @return true If the window has been instructed to close, else false
         */
        bool WindowClosed() const override;

#ifdef SJ_VULKAN_SUPPORT
        /**
         * @return Extensions Vulkan API must support to support this window.   
         */
        Vector<const char*> GetRequiredVulkanExtenstions() const;

        /**
         * @return The Vulkan presentation surface for this window
         */
        VkSurfaceKHR CreateWindowSurface(VkInstance instance) const;
#endif

      private:
        GLFWwindow* m_NativeWindow;
    };

} // namespace sj
