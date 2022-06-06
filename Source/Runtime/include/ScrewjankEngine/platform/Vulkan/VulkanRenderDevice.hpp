#pragma once

// STD Headers

// Library Headers
#include <vulkan/vulkan.h>

// Screwjank Headers
#include <ScrewjankEngine/rendering/RenderDevice.hpp>
#include <ScrewjankEngine/containers/Optional.hpp>

namespace sj {

    // Forward Declarations
    class VulkanRendererAPI;

    struct DeviceQueueFamilyIndices
    {
        Optional<uint32_t> GraphicsFamilyIndex;
        Optional<uint32_t> PresentationFamilyIndex;
    };

    class VulkanRenderDevice : public RenderDevice
    {
      public:
        /**
         * Returns the queue families available for the supplied VkPhysicalDevice
         */
          static DeviceQueueFamilyIndices GetDeviceQueueFamilyIndices(VkPhysicalDevice device,
                                                                      VkSurfaceKHR renderSurface = VK_NULL_HANDLE);

        VulkanRenderDevice() = default;
        ~VulkanRenderDevice() = default;

        void Init(VkSurfaceKHR renderSurface);
        void DeInit();

        /**
         * @return The vulkan handle for the logical device
         */
        VkPhysicalDevice GetPhysicalDevice() const;

        /**
         * @return The vulkan handle for the logical device  
         */
        VkDevice GetLogicalDevice() const;

      private:
        /**
         * Iterates over the system's rendering hardware, and selects the most suitable GPU for
         * rendering
         */
        void SelectPhysicalDevice(VkSurfaceKHR renderSurface);
        
        /**
         * After selecting a physical device, construct the corresponding logical device  
         */
        void CreateLogicalDevice(VkSurfaceKHR renderSurface);

        /**
         * Queries physical device suitability for use in engine
         */
        static bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR renderSurface);

        /** Vulkan's logical representation of the physical device */
        VkDevice m_Device = VK_NULL_HANDLE;

        /** Queue used to handle graphics commands */
        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;

        /** Used to execute presentation commands */
        VkQueue m_PresentationQueue = VK_NULL_HANDLE;

        /**
         * Vulkan's representation of the physical rendering device
         * @note This handle is freed automatically when the VkInstance is destroyed
         */
        VkPhysicalDevice m_PhysicalDevice;

    private:
        bool m_IsInitialized = false;
    };

} // namespace sj
