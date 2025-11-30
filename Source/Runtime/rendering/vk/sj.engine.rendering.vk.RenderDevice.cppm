module;
#include <ScrewjankStd/Assert.hpp>

#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

export module sj.engine.rendering.vk.RenderDevice;
import sj.std.memory;
import sj.engine.rendering.vk.Primitives;

import vulkan_hpp;

export namespace sj::vulkan
{
struct RenderDevice
{
    RenderDevice() = default;
    ~RenderDevice()
    {
        if(mAllocator)
            vmaDestroyAllocator(mAllocator);
    }

    vk::raii::PhysicalDevice mPhysicalDevice {nullptr};
    vk::raii::Device mLogicalDevice {nullptr};

    /** Manages device's memory */
    VmaAllocator mAllocator {};

    /** Queue used to handle graphics commands */
    vk::Queue mGraphicsQueue = VK_NULL_HANDLE;
    uint32_t mGraphicsQueueIndex = -1;

    /** Used to execute presentation commands */
    vk::Queue mPresentationQueue = VK_NULL_HANDLE;
    uint32_t mPresentationQueueIndex = -1;
};
} // namespace sj::vulkan