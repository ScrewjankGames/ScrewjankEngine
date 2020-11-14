// STD Headers

// Library Headers
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// Screwjank Headers
#include "platform/Vulkan/VulkanRendererAPI.hpp"

namespace sj {
    VulkanRendererAPI::VulkanRendererAPI()
    {
        VkApplicationInfo app_info;
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pNext = nullptr;
        app_info.pApplicationName = "Screwjank Engine Game";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "Screwjank Engine";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_2;

        VkInstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = {};
        create_info.pApplicationInfo = &app_info;
        create_info.enabledLayerCount = {};
        create_info.ppEnabledLayerNames = nullptr;

        // Get extension count and names
        auto extenstions = GetRequiredExtenstions();
        create_info.enabledExtensionCount = (uint32_t)extenstions.Size();
        create_info.ppEnabledExtensionNames = extenstions.Data();

        // Compile-time check for adding validation layers
        if constexpr (g_IsDebugBuild) {
            static Vector<const char*> layers(MemorySystem::GetDefaultAllocator(),
                                              {"VK_LAYER_KHRONOS_validation"});
            EnableValidationLayers(layers);

            create_info.enabledLayerCount = (uint32_t)layers.Size();
            create_info.ppEnabledLayerNames = layers.Data();
        }

        // Create the vulkan instance
        VkResult result = vkCreateInstance(&create_info, nullptr, &m_VkInstance);

        if (result != VK_SUCCESS) {
            SJ_ENGINE_LOG_ERROR("Vulkan instance creation failed with error code {}", result);
        }

        if constexpr (g_IsDebugBuild) {
            EnableDebugMessaging();
        }

        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        SJ_ENGINE_LOG_INFO("Vulkan loaded with {} extensions supported", extensionCount);
    }

    VulkanRendererAPI::~VulkanRendererAPI()
    {
        if constexpr (g_IsDebugBuild) {
            auto messenger_destroy_func = (PFN_vkDestroyDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr(m_VkInstance, "vkDestroyDebugUtilsMessengerEXT");

            SJ_ASSERT(messenger_destroy_func != nullptr,
                      "Failed to load Vulkan Debug messenger destroy function";)

            messenger_destroy_func(m_VkInstance, m_VkDebugMessenger, nullptr);
        }

        vkDestroyInstance(m_VkInstance, nullptr);
    }

    Vector<const char*> VulkanRendererAPI::GetRequiredExtenstions() const
    {
        uint32_t extension_count = 0;
        const char** extensions;
        extensions = glfwGetRequiredInstanceExtensions(&extension_count);

        Vector<const char*> extensions_vector;
        extensions_vector.Reserve(extension_count);

        for (size_t i = 0; i < extension_count; i++) {
            extensions_vector.PushBack(extensions[i]);
        }

        if constexpr (g_IsDebugBuild) {
            extensions_vector.PushBack(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions_vector;
    }

    void
    VulkanRendererAPI::EnableValidationLayers(const Vector<const char*>& required_validation_layers)
    {
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        std::vector<VkLayerProperties> available_layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

        // Verify required validation layers are supported
        for (auto layer_name : required_validation_layers) {
            bool layer_found = false;

            for (auto layer_properties : available_layers) {
                if (strcmp(layer_name, layer_properties.layerName) == 0) {
                    layer_found = true;
                    break;
                }
            }

            if (!layer_found) {
                SJ_ASSERT(false, "Failed to enable vulkan validation layer {}", layer_name);
            }

            SJ_ENGINE_LOG_INFO("Enabled Vulkan validation layer: {}", layer_name);
        }
    }

    void VulkanRendererAPI::EnableDebugMessaging()
    {
        // Hook up the debug messenger to instance
        VkDebugUtilsMessengerCreateInfoEXT messenger_create_info = {};
        messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        messenger_create_info.pfnUserCallback = VulkanDebugLogCallback;
        messenger_create_info.pUserData = nullptr;

        // Get extension function pointer
        auto create_function = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            m_VkInstance,
            "vkCreateDebugUtilsMessengerEXT");

        SJ_ASSERT(create_function != nullptr,
                  "Failed to load vulkan extension function vkCreateDebugUtilsMessengerEXT");

        create_function(m_VkInstance, &messenger_create_info, nullptr, &m_VkDebugMessenger);
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRendererAPI::VulkanDebugLogCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
        void* user_data)
    {
        if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
            SJ_ENGINE_LOG_TRACE("Vulkan Validation Layer message: {}", callback_data->pMessage);
        } else if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            SJ_ENGINE_LOG_INFO("Vulkan Validation Layer message: {}", callback_data->pMessage);
        } else if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            SJ_ENGINE_LOG_WARN("Vulkan Validation Layer message: {}", callback_data->pMessage);
        } else if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            SJ_ENGINE_LOG_ERROR("Vulkan Validation Layer message: {}", callback_data->pMessage);
        }

        return VK_FALSE;
    }
} // namespace sj
