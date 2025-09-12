module;

#include <ScrewjankStd/Assert.hpp>

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

#include <array>

export module sj.engine.rendering.vk.FrameData;
import sj.engine.rendering.vk.RenderDevice;
import sj.engine.rendering.vk.Buffer;
import sj.engine.rendering.vk.Primitives;

export namespace sj::vk
{
    constexpr uint32_t kMaxFramesInFlight = 2;

    struct FrameGlobals
    {
        VkCommandBuffer cmd;
        VkFence fence;
        VkSemaphore imgAvailableSemaphore;
        sj::vk::BufferResource globalUniformBuffer;
        VkDescriptorSet globalDescriptorSet;
    };

    struct RenderFrameData
    {
        std::array<VkCommandBuffer, kMaxFramesInFlight> commandBuffers = {};

        std::array<VkSemaphore, kMaxFramesInFlight> imageAvailableSemaphores = {};
        std::array<VkFence, kMaxFramesInFlight> inFlightFences = {};

        std::array<sj::vk::BufferResource, kMaxFramesInFlight> globalUniformBuffers;
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

            // Create Sync Primitives
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
                                                     &imageAvailableSemaphores[i]);
                    SJ_ASSERT(res == VK_SUCCESS, "Failed to create syncronization primitive");

                    res = vkCreateFence(device,
                                        &fenceInfo,
                                        sj::g_vkAllocationFns,
                                        &inFlightFences[i]);
                    SJ_ASSERT(res == VK_SUCCESS, "Failed to create syncronization primitive");
                }
            }
        }

        void DeInit(sj::vk::RenderDevice& device)
        {
            // NOTE: Command buffers are freed for us when we free the command pool.
            //       We only need to clean up sync primitives

            VkDevice logicalDevice = device.GetLogicalDevice();
            VmaAllocator allocator = device.GetAllocator();
            for(uint32_t i = 0; i < kMaxFramesInFlight; i++)
            {
                vkDestroySemaphore(logicalDevice,
                                   imageAvailableSemaphores[i],
                                   sj::g_vkAllocationFns);
                vkDestroyFence(logicalDevice, inFlightFences[i], sj::g_vkAllocationFns);

                globalUniformBuffers[i].DeInit(allocator);
            }
        }

        [[nodiscard]] FrameGlobals GetFrameGlobals(int frameIdx)
        {
            return FrameGlobals
            {
                .cmd = commandBuffers[frameIdx],
                .fence = inFlightFences[frameIdx],
                .imgAvailableSemaphore = imageAvailableSemaphores[frameIdx],
                .globalUniformBuffer = globalUniformBuffers[frameIdx],
                .globalDescriptorSet = globalUBODescriptorSets[frameIdx]
            };
        }
    };
}