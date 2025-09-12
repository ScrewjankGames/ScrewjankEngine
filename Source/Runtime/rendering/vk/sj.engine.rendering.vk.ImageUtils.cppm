module;

#include <ScrewjankStd/Assert.hpp>

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

export module sj.engine.rendering.vk.ImageUtils;
import sj.engine.rendering.vk.Helpers;
import sj.engine.rendering.vk.Primitives;

export namespace sj::vk
{
    class ImageResource
    {
    public:
        ImageResource() = default;
        ImageResource(VmaAllocator allocator,
                      const VmaAllocationCreateInfo& allocationInfo,
                      VkExtent3D extent,
                      VkFormat format,
                      VkImageUsageFlags usageFlags,
                      VkImageTiling tiling)
        {
            Init(allocator, allocationInfo, extent, format, usageFlags, tiling);
        }

        void Init(VmaAllocator allocator,
                  const VmaAllocationCreateInfo& allocationInfo,
                  VkExtent3D extent,
                  VkFormat format,
                  VkImageUsageFlags usageFlags,
                  VkImageTiling tiling)
        {
            VkImageCreateInfo imageInfo {};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = extent.width;
            imageInfo.extent.height = extent.height;
            imageInfo.extent.depth = extent.depth;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = format;
            imageInfo.tiling = tiling;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = usageFlags;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VkResult res = vmaCreateImage(allocator,
                                          &imageInfo,
                                          &allocationInfo,
                                          &m_image,
                                          &m_allocation,
                                          nullptr);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to allocate image");
            m_imageExtent = extent;
            m_imageFormat = format;
        }

        void DeInit(VmaAllocator allocator)
        {
            vmaDestroyImage(allocator, m_image, m_allocation);
        }

        [[nodiscard]] VkImageView MakeImageView(VkDevice device,
                                                VkImageAspectFlags aspectFlags) const
        {
            VkImageViewCreateInfo imageViewCreateInfo =
                sj::vk::MakeImageViewCreateInfo(m_imageFormat, m_image, aspectFlags);

            VkImageView view;
            VkResult res =
                vkCreateImageView(device, &imageViewCreateInfo, g_vkAllocationFns, &view);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to make image view");

            return view;
        }

        [[nodiscard]] VkImage GetImage() const
        {
            return m_image;
        }

        [[nodiscard]] VkExtent2D GetExtent2D() const
        {
            return VkExtent2D{ .width = m_imageExtent.width, .height = m_imageExtent.height };
        }

        [[nodiscard]] VkExtent3D GetExtent() const
        {
            return m_imageExtent;
        }

        [[nodiscard]] VkFormat GetImageFormat() const
        {
            return m_imageFormat;
        }

    private:
        VkImage m_image = VK_NULL_HANDLE;
        VmaAllocation m_allocation = {};
        VkExtent3D m_imageExtent = {};
        VkFormat m_imageFormat = {};
    };

    VkImageSubresourceRange MakeImageSubResourceRange(VkImageAspectFlags aspectMask)
    {
        VkImageSubresourceRange subImage {};
        subImage.aspectMask = aspectMask;
        subImage.baseMipLevel = 0;
        subImage.levelCount = VK_REMAINING_MIP_LEVELS;
        subImage.baseArrayLayer = 0;
        subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;

        return subImage;
    }

    void TransitionImage(VkCommandBuffer cmd,
                         VkImage image,
                         VkImageLayout currentLayout,
                         VkImageLayout newLayout)
    {
        VkImageMemoryBarrier2 imageBarrier {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
        imageBarrier.pNext = nullptr;

        imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

        imageBarrier.oldLayout = currentLayout;
        imageBarrier.newLayout = newLayout;

        VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
                                            ? VK_IMAGE_ASPECT_DEPTH_BIT
                                            : VK_IMAGE_ASPECT_COLOR_BIT;

        imageBarrier.subresourceRange = MakeImageSubResourceRange(aspectMask);
        imageBarrier.image = image;

        VkDependencyInfo depInfo {};
        depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        depInfo.pNext = nullptr;

        depInfo.imageMemoryBarrierCount = 1;
        depInfo.pImageMemoryBarriers = &imageBarrier;

        vkCmdPipelineBarrier2(cmd, &depInfo);
    }

    void CopyBufferToImage(
        VkCommandBuffer cmd, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        VkBufferImageCopy region {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {.x = 0, .y = 0, .z = 0};
        region.imageExtent = {.width = width, .height = height, .depth = 1};

        // "Assuming here that the image has already been transitioned to the layout that is
        // optimal for copying pixels to"
        vkCmdCopyBufferToImage(cmd,
                               buffer,
                               image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1,
                               &region);
    }

    void CopyImageToImage(
        VkCommandBuffer cmd, VkImage source, VkImage dest, VkExtent2D srcSize, VkExtent2D dstSize)
    {
        VkImageBlit2 blitRegion {.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr};

        blitRegion.srcOffsets[1].x = int32_t(srcSize.width);
        blitRegion.srcOffsets[1].y = int32_t(srcSize.height);
        blitRegion.srcOffsets[1].z = 1;

        blitRegion.dstOffsets[1].x = int32_t(dstSize.width);
        blitRegion.dstOffsets[1].y = int32_t(dstSize.height);
        blitRegion.dstOffsets[1].z = 1;

        blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.srcSubresource.baseArrayLayer = 0;
        blitRegion.srcSubresource.layerCount = 1;
        blitRegion.srcSubresource.mipLevel = 0;

        blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.dstSubresource.baseArrayLayer = 0;
        blitRegion.dstSubresource.layerCount = 1;
        blitRegion.dstSubresource.mipLevel = 0;

        VkBlitImageInfo2 blitInfo {.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr};
        blitInfo.dstImage = dest;
        blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        blitInfo.srcImage = source;
        blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        blitInfo.filter = VK_FILTER_LINEAR;
        blitInfo.regionCount = 1;
        blitInfo.pRegions = &blitRegion;

        vkCmdBlitImage2(cmd, &blitInfo);
    }
} // namespace sj::vk