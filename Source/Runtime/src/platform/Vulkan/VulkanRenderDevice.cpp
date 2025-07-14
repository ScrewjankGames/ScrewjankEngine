// Parent Include
#include <ScrewjankEngine/platform/Vulkan/VulkanRenderDevice.hpp>

// STD Headers

// Library Headers
#include <vulkan/vulkan_core.h>

// Screwjank Headers
#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankEngine/rendering/Renderer.hpp>
#include <ScrewjankStd/PlatformDetection.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanHelpers.hpp>

import sj.std.containers.array;
import sj.std.containers.set;
import sj.std.memory;

namespace sj {
    
    /** List of extensions devices must support */
    static constexpr std::array<const char*, 1> kRequiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

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
            vkDestroyDevice(m_Device, sj::g_vkAllocationFns);
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

    VkQueue VulkanRenderDevice::GetGraphicsQueue() const
    {
        return m_GraphicsQueue;
    }

    VkQueue VulkanRenderDevice::GetPresentationQueue() const
    {
        return m_PresentationQueue;
    }

    bool VulkanRenderDevice::IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR renderSurface)
    {
        DeviceQueueFamilyIndices indices = GetDeviceQueueFamilyIndices(device, renderSurface);

        // Query queue support
        bool indicies_complete =
            indices.graphicsFamilyIndex.has_value() && indices.presentationFamilyIndex.has_value();

        static_set<std::string_view, kRequiredDeviceExtensions.size()> missing_extensions(
            kRequiredDeviceExtensions.begin(),
            kRequiredDeviceExtensions.end()
        );

        // Check extension support
        {
            uint32_t extension_count = 0;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
            
            dynamic_array<VkExtensionProperties> extension_props(extension_count, Renderer::WorkBuffer());

            vkEnumerateDeviceExtensionProperties(device,
                                                nullptr,
                                                &extension_count,
                                                extension_props.data());

            for(const VkExtensionProperties& extension : extension_props)
            {
                missing_extensions.erase(extension.extensionName);
            }
        }

        // Check swap chain support
        VulkanSwapChain::SwapChainParams params =
            VulkanSwapChain::QuerySwapChainParams(device, renderSurface);
        bool swap_chain_supported = (params.Formats.size() > 0) && (params.PresentModes.size() > 0);


        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
        
        return indicies_complete && missing_extensions.size() == 0 && swap_chain_supported &&
               supportedFeatures.samplerAnisotropy;
    }


    void VulkanRenderDevice::SelectPhysicalDevice(VkSurfaceKHR renderSurface)
    {
        Renderer* vulkanAPI = Renderer::GetInstance();
        VkInstance instance = vulkanAPI->GetVkInstanceHandle();
        
        uint32_t deviceCount = 0;
        
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        SJ_ENGINE_LOG_INFO("{} Vulkan-capable render devices detected", deviceCount);

        static_vector<VkPhysicalDevice, 4> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
        
        int best_score = -1; 

        // Picks the best compatible GPU found, prefering discrete GPUs  
        for(const VkPhysicalDevice& device : devices)
        {
            int score = 0;

            if(!IsDeviceSuitable(device, renderSurface))
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
            else if (deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            {
                score += 500;
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
        SJ_ASSERT(m_PhysicalDevice != VK_NULL_HANDLE, "CreateLogicalDevice requires a selected physical device");
        SJ_ASSERT(m_Device == VK_NULL_HANDLE, "Logical device already created.");
    
        DeviceQueueFamilyIndices indices =
            GetDeviceQueueFamilyIndices(m_PhysicalDevice, renderSurface);

        constexpr int kMaxUniqueQueues = 2;
        static_set<uint32_t, kMaxUniqueQueues> unique_queue_families;
        unique_queue_families = 
        {
            indices.graphicsFamilyIndex.value(),
            indices.presentationFamilyIndex.value(),
        };

        static_vector<VkDeviceQueueCreateInfo, kMaxUniqueQueues> queue_create_infos;
        float queue_priorities = 1.0f;
        for (auto family : unique_queue_families)
        {
            VkDeviceQueueCreateInfo queue_create_info {};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = family;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priorities;
            queue_create_infos.emplace_back(queue_create_info);
        }

        VkPhysicalDeviceFeatures device_features = {};
        device_features.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo device_create_info = {};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        device_create_info.pQueueCreateInfos = queue_create_infos.data();
        device_create_info.enabledLayerCount = 0;
        device_create_info.pEnabledFeatures = &device_features;
        
        const char* const* deviceExtensionNames = kRequiredDeviceExtensions.data();
        device_create_info.enabledExtensionCount = 
            static_cast<uint32_t>(kRequiredDeviceExtensions.size());
        device_create_info.ppEnabledExtensionNames = deviceExtensionNames;

        VkResult success = vkCreateDevice(
            m_PhysicalDevice, 
            &device_create_info, 
            sj::g_vkAllocationFns, 
            &m_Device
        );
        
        SJ_ASSERT(success == VK_SUCCESS, "Vulkan failed to create logical device.");

        vkGetDeviceQueue(m_Device, *indices.graphicsFamilyIndex, 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, *indices.presentationFamilyIndex, 0, &m_PresentationQueue);
    }

    DeviceQueueFamilyIndices
    VulkanRenderDevice::GetDeviceQueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR renderSurface)
    {
        uint32_t queue_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, nullptr);

        dynamic_array<VkQueueFamilyProperties> queue_data(queue_count, MemorySystem::GetRootMemoryResource());
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, queue_data.data());

        DeviceQueueFamilyIndices indices;

        int i = 0;
        for(const VkQueueFamilyProperties& family : queue_data)
        {
            if(family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamilyIndex = i;
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
                    indices.presentationFamilyIndex = i;
                }
            }

            i++;
        }

        return indices;
    }

} // namespace sj
