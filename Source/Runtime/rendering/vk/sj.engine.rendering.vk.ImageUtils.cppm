module;

#include <ScrewjankStd/Assert.hpp>

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>
#include <utility>

export module sj.engine.rendering.vk.ImageUtils;
import sj.engine.rendering.vk.Helpers;
import sj.engine.rendering.vk.Primitives;

import vulkan_hpp;

export namespace sj::vulkan
{
class ImageResource
{
public:
    ImageResource() = default;
    ImageResource(VmaAllocator allocator,
                  const VmaAllocationCreateInfo& allocationInfo,
                  vk::Extent3D extent,
                  vk::Format format,
                  vk::ImageUsageFlags usageFlags,
                  vk::ImageTiling tiling)
        : mAllocator(allocator), mImageExtent(extent), mImageFormat(format)
    {
        vk::ImageCreateInfo imageInfo({},
                                      vk::ImageType::e2D,
                                      format,
                                      extent,
                                      1,
                                      1,
                                      vk::SampleCountFlagBits::e1,
                                      tiling,
                                      usageFlags,
                                      vk::SharingMode::eExclusive);

        VkImage image;
        VkImageCreateInfo info(imageInfo);
        VkResult res =
            vmaCreateImage(allocator, &info, &allocationInfo, &image, &mAllocation, nullptr);
        SJ_ASSERT(res == VK_SUCCESS, "Failed to allocate image");

        mImage = image;
    }

    ~ImageResource()
    {
        if(mImage)
            vmaDestroyImage(mAllocator, mImage, mAllocation);
    }

    ImageResource(const ImageResource&) = delete;
    ImageResource(ImageResource&& other) noexcept
    {
        *this = std::move(other);
    }

    ImageResource& operator=(ImageResource&& other) noexcept
    {
        mImage = std::exchange(other.mImage, {});
        mImageFormat = std::exchange(other.mImageFormat, {});
        mImageExtent = std::exchange(other.mImageExtent, {});
        mAllocator = std::exchange(other.mAllocator, {});
        mAllocation = std::exchange(other.mAllocation, {});

        return *this;
    }

    [[nodiscard]] vk::raii::ImageView MakeImageView(vk::raii::Device& device,
                                                    vk::ImageAspectFlagBits aspectFlags) const
    {
        VkImageViewCreateInfo imageViewCreateInfo =
            sj::vulkan::MakeImageViewCreateInfo(mImageFormat, mImage, aspectFlags);
        return vk::raii::ImageView(device, imageViewCreateInfo, g_vkAllocationCallbacks);
    }

    [[nodiscard]] vk::Image GetImage() const
    {
        return mImage;
    }

    [[nodiscard]] vk::Extent2D GetExtent2D() const
    {
        return VkExtent2D {.width = mImageExtent.width, .height = mImageExtent.height};
    }

    [[nodiscard]] vk::Extent3D GetExtent() const
    {
        return mImageExtent;
    }

    [[nodiscard]] vk::Format GetImageFormat() const
    {
        return mImageFormat;
    }

private:
    VmaAllocator mAllocator {};
    vk::Format mImageFormat = {};
    vk::Extent3D mImageExtent = {};

    VmaAllocation mAllocation = {};
    vk::Image mImage = VK_NULL_HANDLE;
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
    vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
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
} // namespace sj::vulkan