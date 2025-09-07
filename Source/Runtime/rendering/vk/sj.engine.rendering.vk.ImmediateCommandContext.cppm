module;

#include <ScrewjankStd/Assert.hpp>

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

#include <concepts>

export module sj.engine.rendering.vk.ImmediateCommandContext;
import sj.engine.rendering.vk.Helpers;
import sj.engine.rendering.vk.Primitives;

export namespace sj::vk
{
    class ImmediateCommandContext
    {
    public:
        void Init(VkDevice device, uint32_t queueIndex, VkQueue queue)
        {
            m_device = device;
            m_queue = queue;

            VkCommandPoolCreateInfo poolInfo {};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolInfo.queueFamilyIndex = queueIndex;

            VkResult res = vkCreateCommandPool(device, &poolInfo, sj::g_vkAllocationFns, &m_pool);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to create immediate command pool");

            VkCommandBufferAllocateInfo immCommandInfo =
                sj::vk::MakeCommandBufferAllocateInfo(m_pool, 1);
            res = vkAllocateCommandBuffers(m_device, &immCommandInfo, &m_buffer);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to allocate immediate command buffer");

            VkFenceCreateInfo fenceInfo {};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            vkCreateFence(device, &fenceInfo, sj::g_vkAllocationFns, &m_fence);
        }

        void DeInit()
        {
            vkDestroyFence(m_device, m_fence, sj::g_vkAllocationFns);
            vkDestroyCommandPool(m_device, m_pool, sj::g_vkAllocationFns);
        }

        template <class Fn>
            requires std::invocable<Fn, VkCommandBuffer>
        void ImmediateSubmit(Fn&& function)
        {
            VkResult res = vkResetFences(m_device, 1, &m_fence);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to reset immediate render fence!");

            res = vkResetCommandBuffer(m_buffer, 0);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to reset immediate mode command buffer!");

            VkCommandBufferBeginInfo beginInfo =
                sj::vk::MakeCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

            res = vkBeginCommandBuffer(m_buffer, &beginInfo);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to start immediate mode command buffer!");

            std::forward<Fn>(function)(m_buffer);

            res = vkEndCommandBuffer(m_buffer);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to finalize immediate mode command buffer!");

            VkCommandBufferSubmitInfo submitCommandInfo =
                sj::vk::MakeCommandBufferSubmitInfo(m_buffer);
            VkSubmitInfo2 submitInfo = sj::vk::MakeSubmitInfo(&submitCommandInfo, nullptr, nullptr);

            res = vkQueueSubmit2(m_queue, 1, &submitInfo, m_fence);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to submit immediate command buffer!");

            res = vkWaitForFences(m_device, 1, &m_fence, true, 9999999999);
            SJ_ASSERT(res == VK_SUCCESS, "Immediate mode wait for fence timed out!");
        }

    private:
        VkDevice m_device;
        VkQueue m_queue;
        VkFence m_fence;
        VkCommandPool m_pool;
        VkCommandBuffer m_buffer;
    };
} // namespace sj::vk