#pragma once

// STD Headers

// Library Headers
#include <vulkan/vulkan.h>

// Screwjank Headers
#include "containers/Array.hpp"
#include "containers/String.hpp"
#include "containers/Vector.hpp"
#include "platform/PlatformDetection.hpp"
#include "rendering/RendererAPI.hpp"

namespace sj {

    // Forward declarations
    class VulkanRenderDevice;
    class VulkanSwapChain;
    class Window;
    struct DeviceQueueFamilyIndices;

    class VulkanRendererAPI : public RendererAPI
    {
      public:

        /** List of extensions devices must support */
        static constexpr Array<ConstString, 1> kRequiredDeviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        /**
         * Constructor
         */
        VulkanRendererAPI(Window* window);

        /**
         * Destructor
         */
        ~VulkanRendererAPI();

        /**
         * Returns a pointer to the Vulkan render device
         */
        RenderDevice* GetRenderDevice() override;


        /**
         * @return The active VkInstance
         */
        VkInstance GetInstance() const;

        /**
         * @return The window surface
         */
        VkSurfaceKHR GetRenderingSurface() const; 

        /**
         * @return Handle to the physical device being used by the engine  
         */
        VkPhysicalDevice GetPhysicalDevice() const;

        /**
         * @return Handle to the logical device being used by the engine
         */
        VkDevice GetLogicalDevice() const;

        /**
         * Returns the queue families available for the supplied VkPhysicalDevice 
         */
        DeviceQueueFamilyIndices GetDeviceQueueFamilyIndices(VkPhysicalDevice device) const;

        /**
         * Queries physical device suitability for use in engine  
         */
        bool IsDeviceSuitable(VkPhysicalDevice device) const;

      private:
        /** The Vulkan instance is the engine's connection to the vulkan library */
        VkInstance m_VkInstance;

        /** Handle to manage Vulkan's debug callbacks */
        VkDebugUtilsMessengerEXT m_VkDebugMessenger;

        /** Handle to the surface vulkan renders to */
        VkSurfaceKHR m_RenderingSurface;

        /** Owning pointer to the render device used to back API operations */
        UniquePtr<VulkanRenderDevice> m_RenderDevice;

        /** Owning pointer to the swap chain used for image presentation */
        UniquePtr<VulkanSwapChain> m_SwapChain;

        /**
         * Initializes the Vulkan API's instance and debug messaging hooks
         */
        void InitializeVulkan();

        /**
         * Returns list of Vulkan extenstions the renderer should support
         */
        Vector<const char*> GetRequiredExtenstions() const;

        /** 
         * Communicates with the Window to creates the rendering surface 
         */
        void CreateRenderSurface();

        /**
         * Turns on Vulkan validation layers
         */
        void EnableValidationLayers(const Vector<const char*>& required_validation_layers);

        /**
         * Enables Vulkan Debug messaging
         */
        void EnableDebugMessaging();

        /**
         * Callback function that allows the Vulkan API to use the engine's logging system
         * @note See Vulkan API for description of arguments
         */
        static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugLogCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT severity,
            VkDebugUtilsMessageTypeFlagsEXT message_type,
            const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
            void* user_data
        );
    };

} // namespace sj
