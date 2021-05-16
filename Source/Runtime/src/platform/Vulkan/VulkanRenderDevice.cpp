// STD Headers

// Library Headers

// Screwjank Headers
#include "platform/Vulkan/VulkanRenderDevice.hpp"

#include "containers/Vector.hpp"
#include "containers/UnorderedSet.hpp"
#include "platform/Vulkan/VulkanRendererAPI.hpp"
#include "platform/PlatformDetection.hpp"

namespace sj {

    VulkanRenderDevice::VulkanRenderDevice(VulkanRendererAPI* api)
        : m_PhysicalDevice(VK_NULL_HANDLE), m_API(api)
    {
        SJ_ASSERT(api != nullptr,
                  "Render device did not receive a valid vulkan api");

        SelectPhysicalDevice();
        CreateLogicalDevice();
    }

    VulkanRenderDevice::~VulkanRenderDevice()
    {
        vkDestroyDevice(m_Device, nullptr);
    }

    VkPhysicalDevice VulkanRenderDevice::GetPhysicalDevice() const
    {
        return m_PhysicalDevice;
    }

    VkDevice VulkanRenderDevice::GetLogicalDevice() const
    {
        return m_Device;
    }

    void VulkanRenderDevice::SelectPhysicalDevice()
    {
        VkInstance instance = m_API->GetInstance();
        
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
        SJ_ENGINE_LOG_INFO("{} Vulkan-capable render devices detected", device_count);

        Vector<VkPhysicalDevice> devices(MemorySystem::GetDefaultAllocator(), device_count);
        vkEnumeratePhysicalDevices(instance, &device_count, devices.Data());
        
        int best_score = -1; 

        // Picks the best compatible GPU found, prefering discrete GPUs  
        for (const auto& device : devices) {
            int score = 0;

            if (!m_API->IsDeviceSuitable(device))
            {
                continue;
            }

            VkPhysicalDeviceProperties device_props;
            VkPhysicalDeviceFeatures device_features;

            vkGetPhysicalDeviceProperties(device, &device_props);
            vkGetPhysicalDeviceFeatures(device, &device_features);

            if (device_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                score += 1000;
            }

            if (score > best_score) {
                best_score = score;
                m_PhysicalDevice = device;
            }
        }

        SJ_ASSERT(m_PhysicalDevice != VK_NULL_HANDLE,
                  "Screwjank Engine failed to select suitable physical device.");
    }

    void VulkanRenderDevice::CreateLogicalDevice()
    {
        SJ_ASSERT(m_PhysicalDevice != VK_NULL_HANDLE, "CreateLogicalDevice requires a selected physical device");
        SJ_ASSERT(m_Device == VK_NULL_HANDLE, "Logical device already created.");
    
        DeviceQueueFamilyIndices indices = m_API->GetDeviceQueueFamilyIndices(m_PhysicalDevice);

        UnorderedSet<uint32_t> unique_queue_families(MemorySystem::GetDefaultAllocator());
        unique_queue_families = 
        {
            indices.GraphicsFamilyIndex.Value(),
            indices.PresentationFamilyIndex.Value()
        };

        Vector<VkDeviceQueueCreateInfo> queue_create_infos(MemorySystem::GetDefaultAllocator());
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
        
        const char** deviceExtensionNames = (const char**)(VulkanRendererAPI::kRequiredDeviceExtensions.Data());
        device_create_info.enabledExtensionCount = 
            static_cast<uint32_t>(VulkanRendererAPI::kRequiredDeviceExtensions.Size());
        device_create_info.ppEnabledExtensionNames = deviceExtensionNames;

        VkResult success = vkCreateDevice(m_PhysicalDevice, &device_create_info, nullptr, &m_Device);
        SJ_ASSERT(success == VK_SUCCESS, "Vulkan failed to create logical device.");

        vkGetDeviceQueue(m_Device, *indices.GraphicsFamilyIndex, 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, *indices.PresentationFamilyIndex, 0, &m_PresentationQueue);
    }

} // namespace sj
