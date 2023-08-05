// Parent Include
#include <ScrewjankEngine/platform/Vulkan/VulkanRenderDevice.hpp>

// STD Headers

// Library Headers

// Screwjank Headers
#include <ScrewjankEngine/platform/Vulkan/VulkanRendererAPI.hpp>

#include <ScrewjankEngine/containers/Vector.hpp>
#include <ScrewjankEngine/containers/UnorderedSet.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanRendererAPI.hpp>
#include <ScrewjankEngine/platform/PlatformDetection.hpp>


namespace sj {
    
    /** List of extensions devices must support */
    static constexpr Array<ConstString, 1> kRequiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    void VulkanRenderDevice::Init(VkSurfaceKHR renderSurface)
    {
        SelectPhysicalDevice(renderSurface);
        CreateLogicalDevice(renderSurface);

        m_IsInitialized = true;
    }

    void VulkanRenderDevice::DeInit()
    {
        if(m_Device != VK_NULL_HANDLE)
        {
            vkDestroyDevice(m_Device, nullptr);
        }
    }

    VkPhysicalDevice VulkanRenderDevice::GetPhysicalDevice() const
    {
        SJ_ASSERT(m_IsInitialized, "Accessing uninitialized render device");
        return m_PhysicalDevice;
    }

    VkDevice VulkanRenderDevice::GetLogicalDevice() const
    {
        SJ_ASSERT(m_IsInitialized, "Accessing uninitialized render device");
        return m_Device;
    }

    
    bool VulkanRenderDevice::IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR renderSurface)
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
        vkEnumerateDeviceExtensionProperties(device,
                                             nullptr,
                                             &extension_count,
                                             extension_props.Data());

        UnorderedSet<ConstString> missing_extensions(MemorySystem::GetRootHeapZone(),
                                                     kRequiredDeviceExtensions.begin(),
                                                     kRequiredDeviceExtensions.end());

        for(const VkExtensionProperties& extension : extension_props)
        {
            missing_extensions.Erase(extension.extensionName);
        }

        // Check swap chain support
        VulkanSwapChain::SwapChainParams params =
            VulkanSwapChain::QuerySwapChainParams(device, renderSurface);
        bool swap_chain_supported = !params.Formats.Empty() && !params.PresentModes.Empty();

        return indicies_complete && missing_extensions.Count() == 0 && swap_chain_supported;
    }


    void VulkanRenderDevice::SelectPhysicalDevice(VkSurfaceKHR renderSurface)
    {
        VulkanRendererAPI* vulkanAPI = VulkanRendererAPI::GetInstance();
        VkInstance instance = vulkanAPI->GetVkInstanceHandle();
        
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        SJ_ENGINE_LOG_INFO("{} Vulkan-capable render devices detected", deviceCount);

        Vector<VkPhysicalDevice> devices(MemorySystem::GetRootHeapZone(), deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.Data());
        
        int best_score = -1; 

        // Picks the best compatible GPU found, prefering discrete GPUs  
        for(const VkPhysicalDevice& device : devices)
        {
            int score = 0;

            if(IsDeviceSuitable(device, renderSurface))
            {
                continue;
            }

            VkPhysicalDeviceProperties deviceProps;
            VkPhysicalDeviceFeatures deviceFeatures;

            vkGetPhysicalDeviceProperties(device, &deviceProps);
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

            if (deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) 
            {
                score += 1000;
            }

            if (score > best_score) {
                best_score = score;
                m_PhysicalDevice = device;
            }
        }

        if constexpr(g_IsDebugBuild)
        {
            VkPhysicalDeviceProperties device_props;
            vkGetPhysicalDeviceProperties(m_PhysicalDevice, &device_props);

            SJ_ENGINE_LOG_INFO("Selected render device: {}", device_props.deviceName);
        }

        SJ_ASSERT(m_PhysicalDevice != VK_NULL_HANDLE,
                  "Screwjank Engine failed to select suitable physical device.");
    }

    void VulkanRenderDevice::CreateLogicalDevice(VkSurfaceKHR renderSurface)
    {
        VulkanRendererAPI* vulkanAPI = VulkanRendererAPI::GetInstance();
        SJ_ASSERT(m_PhysicalDevice != VK_NULL_HANDLE, "CreateLogicalDevice requires a selected physical device");
        SJ_ASSERT(m_Device == VK_NULL_HANDLE, "Logical device already created.");
    
        DeviceQueueFamilyIndices indices =
            GetDeviceQueueFamilyIndices(m_PhysicalDevice, renderSurface);

        UnorderedSet<uint32_t> unique_queue_families(MemorySystem::GetRootHeapZone());
        unique_queue_families = 
        {
            indices.GraphicsFamilyIndex.Value(),
            indices.PresentationFamilyIndex.Value()
        };

        Vector<VkDeviceQueueCreateInfo> queue_create_infos(MemorySystem::GetRootHeapZone());
        float queue_priorities = 1.0f;
        for (auto family : unique_queue_families)
        {
            VkDeviceQueueCreateInfo queue_create_info {};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = family;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priorities;
            queue_create_infos.EmplaceBack(queue_create_info);
        }

        VkPhysicalDeviceFeatures device_features = {};

        VkDeviceCreateInfo device_create_info = {};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.Size());
        device_create_info.pQueueCreateInfos = queue_create_infos.Data();
        device_create_info.enabledLayerCount = 0;
        device_create_info.pEnabledFeatures = &device_features;
        
        const char** deviceExtensionNames = (const char**)(kRequiredDeviceExtensions.Data());
        device_create_info.enabledExtensionCount = 
            static_cast<uint32_t>(kRequiredDeviceExtensions.Capacity());
        device_create_info.ppEnabledExtensionNames = deviceExtensionNames;

        VkResult success = vkCreateDevice(m_PhysicalDevice, &device_create_info, nullptr, &m_Device);
        SJ_ASSERT(success == VK_SUCCESS, "Vulkan failed to create logical device.");

        vkGetDeviceQueue(m_Device, *indices.GraphicsFamilyIndex, 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, *indices.PresentationFamilyIndex, 0, &m_PresentationQueue);
    }

    DeviceQueueFamilyIndices
    VulkanRenderDevice::GetDeviceQueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR renderSurface)
    {
        uint32_t queue_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, nullptr);

        Vector<VkQueueFamilyProperties> queue_data(MemorySystem::GetRootHeapZone(), queue_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, queue_data.Data());

        DeviceQueueFamilyIndices indices;

        int i = 0;
        for(const VkQueueFamilyProperties& family : queue_data)
        {
            if(family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.GraphicsFamilyIndex = i;
            }

            if(renderSurface != VK_NULL_HANDLE)
            {
                VkBool32 presentation_support = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device,
                                                     i,
                                                     renderSurface,
                                                     &presentation_support);

                if(presentation_support)
                {
                    indices.PresentationFamilyIndex = i;
                }
            }

            i++;
        }

        return indices;
    }

} // namespace sj
