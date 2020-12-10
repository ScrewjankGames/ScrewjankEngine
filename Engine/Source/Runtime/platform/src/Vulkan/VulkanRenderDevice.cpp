#include "platform/Vulkan/VulkanRenderDevice.hpp"

namespace sj {

    VulkanRenderDevice::VulkanRenderDevice(VkInstance instance)
        : m_PhysicalDevice(VK_NULL_HANDLE), m_VkInstance(instance)
    {
        SJ_ASSERT(m_VkInstance != VK_NULL_HANDLE,
                  "Render device did not receive a valid vulkan instance");
    }

    VulkanRenderDevice::~VulkanRenderDevice()
    {
    }

    void VulkanRenderDevice::SelectPhysicalDevice()
    {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(m_VkInstance, &device_count, nullptr);
        SJ_ENGINE_LOG_INFO("{} Vulkan-capable render devices detected", device_count);
    }

} // namespace sj
