// STD Headers

// Library Headers

// Screwjank Headers
#include "platform/Vulkan/VulkanRenderDevice.hpp"
#include "containers/Vector.hpp"

namespace sj {

    VulkanRenderDevice::VulkanRenderDevice(VkInstance instance)
        : m_PhysicalDevice(VK_NULL_HANDLE), m_VkInstance(instance)
    {
        SJ_ASSERT(m_VkInstance != VK_NULL_HANDLE,
                  "Render device did not receive a valid vulkan instance");

        SelectPhysicalDevice();
    }

    VulkanRenderDevice::~VulkanRenderDevice()
    {
    }

    void VulkanRenderDevice::SelectPhysicalDevice()
    {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(m_VkInstance, &device_count, nullptr);
        SJ_ENGINE_LOG_INFO("{} Vulkan-capable render devices detected", device_count);

        Vector<VkPhysicalDevice> devices(MemorySystem::GetDefaultAllocator(), device_count);
        vkEnumeratePhysicalDevices(m_VkInstance, &device_count, devices.Data());
        
        int best_score = -1; 

        // Picks the first discrete GPU found, if no discrete GPU first Vulkan compatible device
        for (const auto& device : devices) {
            int score = 0;

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
    }

} // namespace sj
