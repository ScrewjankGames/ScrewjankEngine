#pragma once

// STD Headers

// Library Headers
#include <vulkan/vulkan.h>

// Screwjank Headers
#include "rendering/RendererAPI.hpp"
#include "containers/Vector.hpp"

namespace sj {

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

      private:
        /** The Vulkan instance is the engine's connection to the vulkan library */
        VkInstance m_VkInstance;

        /** Handle to manage Vulkan's debug callbacks */
        VkDebugUtilsMessengerEXT m_VkDebugMessenger;

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
