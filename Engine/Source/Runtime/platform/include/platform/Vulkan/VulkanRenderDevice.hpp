#pragma once

// STD Headers

// Library Headers
#include <vulkan/vulkan.h>

// Screwjank Headers
#include "rendering/RenderDevice.hpp"

namespace sj {
    class VulkanRenderDevice : public RenderDevice
    {
      public:
        /**
         * Constructor
         */
        VulkanRenderDevice(VkInstance instance);

        /**
         * Destructor
         */
        ~VulkanRenderDevice();

      private:
        /**
         * Iterates over the system's rendering hardware, and selects the most suitable GPU for
         * rendering
         */
        void SelectPhysicalDevice();
        
        /**
         * Vulkan's logical representation of the physical rendering device
         * @note This handle is freed when the VkInstance is destroyed
         */
        VkPhysicalDevice m_PhysicalDevice;

        /** Non-owning handle to the Vulkan instance */
        VkInstance m_VkInstance;
    };
} // namespace sj
