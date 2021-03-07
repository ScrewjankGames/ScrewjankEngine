// STD Headers
#include <unordered_set>

// Library Headers

// Screwjank Headers
#include "platform/Vulkan/VulkanRendererAPI.hpp"

#include "core/Window.hpp"
#include "containers/String.hpp"
#include "containers/UnorderedSet.hpp"
#include "platform/Vulkan/VulkanRenderDevice.hpp"

#ifdef SJ_PLATFORM_WINDOWS
#include "platform/Windows/WindowsWindow.hpp"
#endif // SJ_PLATFORM_WINDOWS


namespace sj {

    VulkanRendererAPI::VulkanRendererAPI(Window* window) : RendererAPI(window)
    {

        InitializeVulkan();

        // Compile-time check to enable debug messaging
        if constexpr (g_IsDebugBuild) {
            EnableDebugMessaging();
        }

        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        SJ_ENGINE_LOG_INFO("Vulkan loaded with {} extensions supported", extensionCount);

        // Create physical and logical render devices
        m_RenderDevice = MakeUnique<VulkanRenderDevice>(this);
        
        // Create Swap Chain
        CreateSwapChain();
    }

    
    VulkanRendererAPI::~VulkanRendererAPI()
    {
        if constexpr (g_IsDebugBuild)
        {
            auto messenger_destroy_func = (PFN_vkDestroyDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr(m_VkInstance, "vkDestroyDebugUtilsMessengerEXT");

            SJ_ASSERT(messenger_destroy_func != nullptr,
                      "Failed to load Vulkan Debug messenger destroy function");

            messenger_destroy_func(m_VkInstance, m_VkDebugMessenger, nullptr);
        }

        // Important: Device must be destroyed before instance
        m_RenderDevice.Reset();
        vkDestroySurfaceKHR(m_VkInstance, m_RenderingSurface, nullptr);
        vkDestroyInstance(m_VkInstance, nullptr);
    }

    void VulkanRendererAPI::InitializeVulkan()
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
        Vector<const char*> extenstions = GetRequiredExtenstions();
        create_info.enabledExtensionCount = (uint32_t)extenstions.Size();
        create_info.ppEnabledExtensionNames = extenstions.Data();

        // Compile-time check for adding validation layers
        if constexpr (g_IsDebugBuild) {
            static Vector<const char*> layers(
                MemorySystem::GetDefaultAllocator(),
                {"VK_LAYER_KHRONOS_validation"}
            );

            EnableValidationLayers(layers);

            create_info.enabledLayerCount = (uint32_t)layers.Size();
            create_info.ppEnabledLayerNames = layers.Data();
        }


        // Create the vulkan instance
        VkResult result = vkCreateInstance(&create_info, nullptr, &m_VkInstance);
        SJ_ASSERT(result == VK_SUCCESS,
                  "Vulkan instance creation failed with error code {}",
                  result);

        CreateRenderSurface();
    }

    RenderDevice* VulkanRendererAPI::GetRenderDevice()
    {
        return m_RenderDevice.Get();
    }

    DeviceQueueFamilyIndices
    VulkanRendererAPI::GetDeviceQueueFamilyIndices(VkPhysicalDevice device) const
    {
        uint32_t queue_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, nullptr);

        Vector<VkQueueFamilyProperties> queue_data(MemorySystem::GetDefaultAllocator(),
                                                   queue_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, queue_data.Data());

        DeviceQueueFamilyIndices indices;

        int i = 0;
        for (const auto& family : queue_data)
        {
            if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.GraphicsFamilyIndex = i;
            }

            if (m_RenderingSurface != VK_NULL_HANDLE)
            {
                VkBool32 presentation_support = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(
                    device,
                    i,
                    m_RenderingSurface,
                    &presentation_support
                );

                if (presentation_support)
                {
                    indices.PresentationFamilyIndex = i;
                }
            }

            i++;
        }

        return indices;
    }

    bool VulkanRendererAPI::IsDeviceSuitable(VkPhysicalDevice device) const
    {
        DeviceQueueFamilyIndices indices = GetDeviceQueueFamilyIndices(device);

        // Query queue support
        bool indicies_complete =
            indices.GraphicsFamilyIndex.HasValue() && indices.PresentationFamilyIndex.HasValue();

        // Check extension support
        uint32_t extension_count;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
        Vector<VkExtensionProperties> extension_props(MemorySystem::GetDefaultAllocator(), extension_count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, extension_props.Data());

        UnorderedSet<ConstString> missing_extensions(
            MemorySystem::GetDefaultAllocator(),
            kRequiredDeviceExtensions.begin(),
            kRequiredDeviceExtensions.end()
        );

        for (const VkExtensionProperties& extension : extension_props)
        {
            missing_extensions.Erase(extension.extensionName);
        }

        // Check swap chain dupport
        SwapChainParams params = QuerySwapChainParams(device);
        bool swap_chain_supported = !params.Formats.Empty() && !params.PresentModes.Empty();
        

        return indicies_complete && missing_extensions.Count() == 0 && swap_chain_supported;
    }

    VkInstance VulkanRendererAPI::GetInstance() const
    {
        return m_VkInstance;
    }

    VkSurfaceKHR VulkanRendererAPI::GetRenderingSurface() const
    {
        return m_RenderingSurface;
    }

    Vector<const char*> VulkanRendererAPI::GetRequiredExtenstions() const
    {
        Vector<const char*> extensions_vector;
#ifdef SJ_PLATFORM_WINDOWS
        extensions_vector = reinterpret_cast<WindowsWindow*>(m_Window)->GetRequiredVulkanExtenstions();
#else
        #error
#endif 

        if constexpr (g_IsDebugBuild)
        {
            extensions_vector.PushBack(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions_vector;
    }

    void VulkanRendererAPI::CreateRenderSurface()
    {
#ifdef SJ_PLATFORM_WINDOWS
        m_RenderingSurface =
            reinterpret_cast<WindowsWindow*>(m_Window)->CreateWindowSurface(m_VkInstance);
#else
    #error
#endif 
    }

    void VulkanRendererAPI::CreateSwapChain()
    {

    }

    VulkanRendererAPI::SwapChainParams VulkanRendererAPI::QuerySwapChainParams(VkPhysicalDevice physical_device) const
    {
        SwapChainParams params = {};

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device,
                                                  m_RenderingSurface,
                                                  &params.Capabilities);

        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,
                                             m_RenderingSurface,
                                             &format_count,
                                             nullptr);

        SJ_ASSERT(format_count != 0, "No surface formats found");

        params.Formats.Reserve(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,
                                             m_RenderingSurface,
                                             &format_count,
                                             params.Formats.Data());

        uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
                                                  m_RenderingSurface,
                                                  &present_mode_count,
                                                  nullptr);

        SJ_ASSERT(present_mode_count != 0, "No present modes found");
        params.PresentModes.Reserve(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
                                                  m_RenderingSurface,
                                                  &present_mode_count,
                                                  params.PresentModes.Data());

        return params;
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

            for (const auto& layer_properties : available_layers) {
                if (strcmp(layer_name, layer_properties.layerName) == 0) {
                    layer_found = true;
                    break;
                }
            }

            if (!layer_found) {
                SJ_ASSERT(false, "Failed to enable vulkan validation layer {}", layer_name);
            }

            SJ_ENGINE_LOG_DEBUG("Enabled Vulkan validation layer: {}", layer_name);
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
