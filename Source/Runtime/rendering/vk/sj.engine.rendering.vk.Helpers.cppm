module;

#include <vulkan/vulkan_core.h>

export module sj.engine.rendering.vk.Helpers;

export namespace sj::vk
{
    VkCommandPoolCreateInfo MakeCommandPoolCreateInfo(uint32_t queueFamilyIndex,
                                                      VkCommandPoolCreateFlags flags)
    {
        VkCommandPoolCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.pNext = nullptr;
        info.queueFamilyIndex = queueFamilyIndex;
        info.flags = flags;

        return info;
    }

    VkCommandBufferAllocateInfo MakeCommandBufferAllocateInfo(VkCommandPool pool, uint32_t count)
    {
        VkCommandBufferAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.pNext = nullptr;

        info.commandPool = pool;
        info.commandBufferCount = count;
        info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        return info;
    }

    VkFenceCreateInfo MakeFenceCreateInfo(VkFenceCreateFlags flags = 0)
    {
        VkFenceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.pNext = nullptr;

        info.flags = flags;

        return info;
    }

    VkSemaphoreCreateInfo MakeSemaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0)
    {
        VkSemaphoreCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.pNext = nullptr;

        info.flags = flags;

        return info;
    }

} // namespace sj::vk