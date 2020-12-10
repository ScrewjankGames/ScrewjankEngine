#pragma once

// STD Headers

// Library Headers
#include <vulkan/vulkan.h>

// Screwjank Headers
#include "rendering/RendererAPI.hpp"
#include "containers/Vector.hpp"

namespace sj {

    // Forward declarations
    class VulkanRenderDevice;

    class VulkanRendererAPI : public RendererAPI
    {
      public:
        /**
         * Constructor
         */
        VulkanRendererAPI();

        /**
         * Destructor
         */
        ~VulkanRendererAPI();

        /**
         * Returns a pointer to the Vulkan render device
         */
        RenderDevice* GetRenderDevice() override;

      private:
        /** The Vulkan instance is the engine's connection to the vulkan library */
        VkInstance m_VkInstance;

        /** Handle to manage Vulkan's debug callbacks */
        VkDebugUtilsMessengerEXT m_VkDebugMessenger;

        /** Owning pointer to the render device used to back API operations */
        UniquePtr<VulkanRenderDevice> m_RenderDevice;

        /**
         * Initializes the Vulkan API's instance and debug messaging hooks
         */
        void InitializeVulkan();

        /**
         * Returns list of Vulkan extenstions the renderer should support
         */
        Vector<const char*> GetRequiredExtenstions() const;

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
        static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugLogCallback( //
            VkDebugUtilsMessageSeverityFlagBitsEXT severity, //
            VkDebugUtilsMessageTypeFlagsEXT message_type, //
            const VkDebugUtilsMessengerCallbackDataEXT* callback_data, //
            void* user_data //
        );
    };

} // namespace sj
