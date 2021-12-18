// STD Headers
#include <unordered_set>

// Library Headers

// Screwjank Headers
#include <ScrewjankEngine/platform/Vulkan/VulkanRendererAPI.hpp>

#include <ScrewjankEngine/core/Window.hpp>
#include <ScrewjankEngine/containers/String.hpp>
#include <ScrewjankEngine/containers/UnorderedSet.hpp>
#include <ScrewjankEngine/rendering/Renderer.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanRenderDevice.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanSwapChain.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanPipeline.hpp>

#ifdef SJ_PLATFORM_WINDOWS
#include <ScrewjankEngine/platform/Windows/WindowsWindow.hpp>
#endif // SJ_PLATFORM_WINDOWS


namespace sj {

    VulkanRendererAPI::VulkanRendererAPI()
    {
        InitializeVulkan();

        SJ_ASSERT(m_RenderDevice.Get() != nullptr, "Failed to create render device");
        SJ_ASSERT(m_SwapChain.Get() != nullptr, "Failed to create swap chain");
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
    
        // Important: Component Unique pointers must be released in a specific order
        m_SwapChain.Reset();
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
        if constexpr (g_IsDebugBuild) 
        {
            static Vector<const char*> layers( 
                MemorySystem::GetRootHeapZone(),
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

        // Compile-time check to enable debug messaging
        if constexpr (g_IsDebugBuild)
        {
            EnableDebugMessaging();
        }

        // Create rendering surface
        CreateRenderSurface();

        // Select physical device and create and logical render device
        m_RenderDevice = MakeUnique<VulkanRenderDevice>(MemorySystem::GetRootHeapZone(), this);

        // Create the vulkan swap chain connected to the current window
        m_SwapChain = MakeUnique<VulkanSwapChain>(MemorySystem::GetRootHeapZone(), this, Window::GetInstance());

        // Log success
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        SJ_ENGINE_LOG_INFO("Vulkan loaded with {} extensions supported", extensionCount);
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

        Vector<VkQueueFamilyProperties> queue_data(MemorySystem::GetRootHeapZone(),
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
        Vector<VkExtensionProperties> extension_props(MemorySystem::GetRootHeapZone(),
                                                      extension_count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, extension_props.Data());

        UnorderedSet<ConstString> missing_extensions(
            MemorySystem::GetRootHeapZone(),
            kRequiredDeviceExtensions.begin(),
            kRequiredDeviceExtensions.end()
        );

        for (const VkExtensionProperties& extension : extension_props)
        {
            missing_extensions.Erase(extension.extensionName);
        }

        // Check swap chain dupport
        VulkanSwapChain::SwapChainParams params = m_SwapChain->QuerySwapChainParams(device, m_RenderingSurface);
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

    VkPhysicalDevice VulkanRendererAPI::GetPhysicalDevice() const
    {
        return m_RenderDevice->GetPhysicalDevice();
    }

    VkDevice VulkanRendererAPI::GetLogicalDevice() const
    {
        return m_RenderDevice->GetLogicalDevice();
    }

    Vector<const char*> VulkanRendererAPI::GetRequiredExtenstions() const
    {
        Vector<const char*> extensions_vector;

        extensions_vector = Window::GetInstance()->GetRequiredVulkanExtenstions();

        if constexpr (g_IsDebugBuild)
        {
            extensions_vector.PushBack(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions_vector;
    }

    void VulkanRendererAPI::CreateRenderSurface()
    {
        m_RenderingSurface = Window::GetInstance()->CreateWindowSurface(m_VkInstance);
    }

    void
    VulkanRendererAPI::EnableValidationLayers(const Vector<const char*>& required_validation_layers)
    {
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        Vector<VkLayerProperties> available_layers(Renderer::WorkBuffer(), layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.Data());

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
