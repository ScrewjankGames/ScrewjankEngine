module;

#include <ScrewjankStd/Assert.hpp>

#include <vk_mem_alloc.h>

#include <concepts>

export module sj.engine.rendering.vk.ImmediateCommandContext;
import sj.engine.rendering.vk.Helpers;
import sj.engine.rendering.vk.Primitives;

import vulkan_hpp;

export namespace sj::vulkan
{
class ImmediateCommandContext
{
public:
    ImmediateCommandContext() = default;
    ImmediateCommandContext(vk::raii::Device& device, uint32_t queueIndex, vk::Queue queue)
        : mDevice(device), mQueue(queue)
    {
        vk::CommandPoolCreateInfo poolInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                           queueIndex);

        mPool = vk::raii::CommandPool(device, poolInfo, g_vkAllocationCallbacks);

        vk::CommandBufferAllocateInfo immCommandInfo(mPool, vk::CommandBufferLevel::ePrimary, 1);
        
        // Use c-api because c++ one forces use of std::vector even though we know there's only one element 
        {
            VkCommandBufferAllocateInfo cInf = immCommandInfo;
            VkCommandBuffer cBuf;
            VkResult res = vkAllocateCommandBuffers(*device, &cInf, &cBuf);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to allocate immediate command buffer");
            mBuffer = cBuf;
        }

        vk::FenceCreateInfo fenceInfo(vk::FenceCreateFlagBits::eSignaled);
        mFence = vk::raii::Fence(device, fenceInfo, g_vkAllocationCallbacks);
    }

    template <class Fn>
        requires std::invocable<Fn, vk::CommandBuffer>
    void ImmediateSubmit(Fn&& function)
    {
        mDevice.resetFences({mFence});
        mBuffer.reset();

        vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        mBuffer.begin(beginInfo);

        std::forward<Fn>(function)(mBuffer);

        mBuffer.end();

        vk::CommandBufferSubmitInfo submitCommandInfo(mBuffer);
        vk::SubmitInfo2 submitInfo({}, {}, {submitCommandInfo});
        mQueue.submit2({submitInfo}, mFence);
        vk::Result res = mDevice.waitForFences({mFence}, true, 9999999999);
        SJ_ASSERT(res == vk::Result::eSuccess, "Immediate mode wait for fence timed out!");
    }

private:
    vk::raii::Fence mFence {nullptr};
    vk::raii::CommandPool mPool {nullptr};

    vk::Device mDevice;
    vk::Queue mQueue;
    vk::CommandBuffer mBuffer;
};
} // namespace sj::vulkan