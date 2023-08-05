#pragma once

// STD Headers

// Library Headers
#include <vulkan/vulkan.h>

// Screwjank Headers
#include <ScrewjankEngine/containers/Array.hpp>
#include <ScrewjankEngine/containers/String.hpp>
#include <ScrewjankEngine/containers/Vector.hpp>
#include <ScrewjankEngine/platform/PlatformDetection.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanRenderDevice.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanSwapChain.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanPipeline.hpp>
#include <ScrewjankEngine/rendering/RendererAPI.hpp>

namespace sj {

    // Forward declarations
    class VulkanPipeline;
    class VulkanRenderDevice;
    class VulkanSwapChain;
    class Window;
    struct DeviceQueueFamilyIndices;

    class VulkanRendererAPI : public RendererAPI
    {
      public:
        static VulkanRendererAPI* GetInstance();

        /**
         * @return The active VkInstance
         */
        VkInstance GetVkInstanceHandle() const;

      private:

        /**
         * Callback function that allows the Vulkan API to use the engine's logging system
         * @note See Vulkan API for description of arguments
         */
        static VKAPI_ATTR VkBool32 VKAPI_CALL
        VulkanDebugLogCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                               VkDebugUtilsMessageTypeFlagsEXT message_type,
                               const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                               void* user_data);

        /**
         * Constructor
         */
        VulkanRendererAPI() = default;

        /**
         * Destructor
         */
        ~VulkanRendererAPI();

        void Init() override;
        void DeInit() override;

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
         * Creates default render pass 
         */
        void CreateRenderPass();

        /**
         * Turns on Vulkan validation layers
         */
        void EnableValidationLayers(const Vector<const char*>& required_validation_layers);

        /**
         * Enables Vulkan Debug messaging
         */
        void EnableDebugMessaging();

        bool m_IsInitialized = false;

        /** The Vulkan instance is the engine's connection to the vulkan library */
        VkInstance m_VkInstance;

        /** Handle to manage Vulkan's debug callbacks */
        VkDebugUtilsMessengerEXT m_VkDebugMessenger;

        /** Handle to the surface vulkan renders to */
        VkSurfaceKHR m_RenderingSurface;

        /** It's a render pass I guess. */
        VkRenderPass m_DefaultRenderPass;

        /** Used to back API operations */
        VulkanRenderDevice m_RenderDevice;

        /** Used for image presentation */
        VulkanSwapChain m_SwapChain;

        /** Pipeline used to describe rendering process */
        VulkanPipeline m_DefaultPipeline;
    };

} // namespace sj
