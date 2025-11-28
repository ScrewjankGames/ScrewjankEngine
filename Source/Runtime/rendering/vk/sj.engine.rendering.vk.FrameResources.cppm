module;

#include <ScrewjankStd/Assert.hpp>

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

#include <array>

export module sj.engine.rendering.vk.FrameResources;
import sj.engine.rendering.vk.RenderDevice;
import sj.engine.rendering.vk.Buffer;
import sj.engine.rendering.vk.Primitives;

export namespace sj::vulkan
{
constexpr uint32_t kMaxFramesInFlight = 2;

struct FrameGlobals
{
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;
    VkSemaphore presentCompleteSemaphore = VK_NULL_HANDLE;
    VkDescriptorSet globalDescriptorSet = VK_NULL_HANDLE;

    sj::vulkan::BufferResource globalUniformBuffer;
};

struct FrameResources
{
    std::array<VkCommandBuffer, kMaxFramesInFlight> commandBuffers = {};

    std::array<VkFence, kMaxFramesInFlight> inFlightFences = {};
    std::array<VkSemaphore, kMaxFramesInFlight> presentCompleteSemaphores = {};

    std::array<sj::vulkan::BufferResource, kMaxFramesInFlight> globalUniformBuffers;
    std::array<VkDescriptorSet, kMaxFramesInFlight> globalUBODescriptorSets = {};

    void Init(VkDevice device, VkCommandPool commandPool)
    {
        // Create Command Buffer
        {
            VkCommandBufferAllocateInfo allocInfo {};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = commandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

            VkResult res = vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());

            SJ_ASSERT(res == VK_SUCCESS, "Failed to create graphics command buffer");
        }

        CreateSyncPrimitives(device);
    }

    void CreateSyncPrimitives(VkDevice device)
    {
        VkSemaphoreCreateInfo semaphoreInfo {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for(uint32_t i = 0; i < kMaxFramesInFlight; i++)
        {
            VkResult res = vkCreateSemaphore(device,
                                             &semaphoreInfo,
                                             sj::g_vkAllocationFns,
                                             &presentCompleteSemaphores[i]);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to create syncronization primitive");

            res = vkCreateFence(device, &fenceInfo, sj::g_vkAllocationFns, &inFlightFences[i]);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to create syncronization primitive");
        }
    }

    void DestroySyncPrimitives(VkDevice logicalDevice)
    {
        for(uint32_t i = 0; i < kMaxFramesInFlight; i++)
        {
            vkDestroySemaphore(logicalDevice, presentCompleteSemaphores[i], sj::g_vkAllocationFns);
            vkDestroyFence(logicalDevice, inFlightFences[i], sj::g_vkAllocationFns);
        }
    }

    void DeInit(sj::vulkan::RenderDevice& device)
    {
        DestroySyncPrimitives(device.GetLogicalDevice());

        VmaAllocator allocator = device.GetAllocator();
        for(uint32_t i = 0; i < kMaxFramesInFlight; i++)
        {
            globalUniformBuffers[i].DeInit(allocator);
        }
    }

    [[nodiscard]] FrameGlobals GetFrameGlobals(int frameIdx)
    {
        return FrameGlobals {.cmd = commandBuffers[frameIdx],
                             .fence = inFlightFences[frameIdx],
                             .presentCompleteSemaphore = presentCompleteSemaphores[frameIdx],
                             .globalDescriptorSet = globalUBODescriptorSets[frameIdx],
                             .globalUniformBuffer = globalUniformBuffers[frameIdx]};
    }
};
} // namespace sj::vulkan