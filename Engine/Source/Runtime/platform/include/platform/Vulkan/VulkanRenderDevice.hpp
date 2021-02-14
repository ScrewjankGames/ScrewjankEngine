#pragma once

// STD Headers

// Library Headers
#include <vulkan/vulkan.h>

// Screwjank Headers
#include "rendering/RenderDevice.hpp"
#include "containers/Optional.hpp"

namespace sj {
    
    // Forward Declarations
    class VulkanRendererAPI;

    class VulkanRenderDevice : public RenderDevice
    {
      public:

        struct QueueFamilyIndices
        {
            Optional<uint32_t> GraphicsFamilyIndex;
            Optional<uint32_t> PresentationFamilyIndex;
        };

        /**
         * Constructor
         */
        VulkanRenderDevice(VulkanRendererAPI* api);

        /**
         * Destructor
         */
        ~VulkanRenderDevice();

        /**
         * 
         */
        QueueFamilyIndices GetQueueFamilyIndices() const;

      private:
        /**
         * Iterates over the system's rendering hardware, and selects the most suitable GPU for
         * rendering
         */
        void SelectPhysicalDevice();
        
        /**
         * After selecting a physical device, construct the corresponding logical device  
         */
        void CreateLogicalDevice();

        /** Vulkan's logical representation of the physical device */
        VkDevice m_Device;

        /** Queue used to handle graphics commands */
        VkQueue m_GraphicsQueue;

        /** Used to execute presentation commands */
        VkQueue m_PresentationQueue;

        /**
         * Vulkan's representation of the physical rendering device
         * @note This handle is freed automatically when the VkInstance is destroyed
         */
        VkPhysicalDevice m_PhysicalDevice;

        /** Non-owning handle to the API for access to API settings */
        VulkanRendererAPI* m_API;
    };

} // namespace sj
