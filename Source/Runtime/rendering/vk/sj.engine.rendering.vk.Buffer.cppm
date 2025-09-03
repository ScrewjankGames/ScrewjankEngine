module;

#include <ScrewjankStd/Assert.hpp>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

export module sj.engine.rendering.vk.Buffer;
import sj.engine.rendering.vk.RenderDevice;
import sj.engine.rendering.vk.Primitives;

export namespace sj::vk
{
    struct BufferResource
    {
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo info;
    };

    BufferResource CreateBuffer(VmaAllocator allocator,
                                size_t bufferSizeBytes,
                                VkBufferUsageFlags bufferUsage,
                                VmaAllocationCreateFlags allocationFlags = {},
                                VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO)
    {
        // allocate buffer
        VkBufferCreateInfo bufferInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferInfo.pNext = nullptr;
        bufferInfo.size = bufferSizeBytes;
        bufferInfo.usage = bufferUsage;

        VmaAllocationCreateInfo vmaallocInfo = {};
        vmaallocInfo.usage = memoryUsage;
        vmaallocInfo.flags = allocationFlags;

        BufferResource newBuffer;

        // allocate the buffer
        VkResult res = vmaCreateBuffer(allocator,
                                       &bufferInfo,
                                       &vmaallocInfo,
                                       &newBuffer.buffer,
                                       &newBuffer.allocation,
                                       &newBuffer.info);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to allocate buffer");

        return newBuffer;
    }

    void DestroyBuffer(VmaAllocator allocator, BufferResource& buffer)
    {
        vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
    }

} // namespace sj::vk