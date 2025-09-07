module;

#include <ScrewjankStd/Assert.hpp>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

export module sj.engine.rendering.vk.Buffer;

export namespace sj::vk
{
    class BufferResource
    {
    public:
        BufferResource() = default;
        BufferResource(VmaAllocator allocator,
                       VkDeviceSize bufferSizeBytes,
                       VkBufferUsageFlags bufferUsage,
                       VmaAllocationCreateFlags allocationFlags = {},
                       VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO)
        {
            Init(allocator, bufferSizeBytes, bufferUsage, allocationFlags, memoryUsage);
        }

        void Init(VmaAllocator allocator,
                  VkDeviceSize bufferSizeBytes,
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

            // allocate the buffer
            VkResult res =
                vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo, &buffer, &allocation, &info);

            SJ_ASSERT(res == VK_SUCCESS, "Failed to allocate buffer");
        }

        void DeInit(VmaAllocator allocator)
        {
            vmaDestroyBuffer(allocator, buffer, allocation);
        }

        [[nodiscard]] VkBuffer GetBuffer() const
        {
            return buffer;
        }

        [[nodiscard]] void* GetMappedMemory()
        {
            return info.pMappedData;
        }

    private:
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = {};
        VmaAllocationInfo info = {};
    };

    BufferResource MakeStagingBuffer(VmaAllocator allocator, size_t bufferSizeBytes)
    {
        return BufferResource(allocator,
                              bufferSizeBytes,
                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                  VMA_ALLOCATION_CREATE_MAPPED_BIT);
    }

    void CopyBuffer(VkCommandBuffer cmd, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        VkBufferCopy copyRegion {};
        copyRegion.size = size;
        vkCmdCopyBuffer(cmd, srcBuffer, dstBuffer, 1, &copyRegion);
    }
} // namespace sj::vk