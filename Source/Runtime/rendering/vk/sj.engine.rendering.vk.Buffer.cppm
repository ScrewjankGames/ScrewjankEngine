module;

#include <ScrewjankStd/Assert.hpp>
#include <vulkan/vulkan.h>

export module sj.engine.rendering.vk.Buffer;
import sj.engine.rendering.vk.RenderDevice;

export namespace sj::vk
{
    void CreateBuffer(const sj::vk::RenderDevice& device,
                      VkDeviceSize size,
                      VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties,
                      VkBuffer& out_buffer,
                      VkDeviceMemory& out_bufferMemory)
    {
        VkDevice logicalDevice = device.GetLogicalDevice();

        std::array<uint32_t, 1> queueFamilyIndices {device.GetGraphicsQueueIndex()};

        VkBufferCreateInfo bufferInfo {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.queueFamilyIndexCount = 1;
        bufferInfo.pQueueFamilyIndices = queueFamilyIndices.data();

        VkResult res =
            vkCreateBuffer(logicalDevice, &bufferInfo, sj::g_vkAllocationFns, &out_buffer);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to create vulkan buffer");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(logicalDevice, out_buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex =
            FindMemoryType(device.GetPhysicalDevice(), memRequirements.memoryTypeBits, properties);

        res = vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &out_bufferMemory);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to allocate dummy vertex buffer memory");

        vkBindBufferMemory(logicalDevice, out_buffer, out_bufferMemory, 0);
    }
} // namespace sj::vk