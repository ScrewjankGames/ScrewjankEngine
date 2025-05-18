// Parent include
#include "ScrewjankEngine/rendering/Renderer.hpp"
#include <ScrewjankEngine/platform/Vulkan/VulkanHelpers.hpp>

// Screwjank Headers
#include <ScrewjankShared/utils/Log.hpp>
#include <ScrewjankShared/utils/Assert.hpp>

//STD Includes
#include <array>

import sj.engine.system.memory;

namespace sj
{
    uint32_t FindMemoryType(VkPhysicalDevice physicalDevice,
                            uint32_t typeFilter,
                            VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if((typeFilter & (1 << i)) &&
               (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        SJ_ASSERT(false, "Failed to find suitable memory type.");
        return -1;
    }

    void CreateImage(VkDevice logicalDevice, 
                     VkPhysicalDevice physicalDevice,
                     uint32_t width,
                     uint32_t height,
                     VkFormat format,
                     VkImageTiling tiling,
                     VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties,
                     VkImage& image,
                     VkDeviceMemory& imageMemory)
    {
        VkImageCreateInfo imageInfo {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult success = vkCreateImage(
            logicalDevice, 
            &imageInfo, 
            sj::g_vkAllocationFns, 
            &image
        );

        SJ_ASSERT(success == VK_SUCCESS, "failed to create image!");

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

        success = vkAllocateMemory(
            logicalDevice, 
            &allocInfo, 
            sj::g_vkAllocationFns, 
            &imageMemory
        );
        SJ_ASSERT(success == VK_SUCCESS, "failed to allocate image memory!");

        vkBindImageMemory(logicalDevice, image, imageMemory, 0);
    }

    VkImageView
    CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
    {
        VkImageViewCreateInfo viewInfo {};

        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        VkResult res = vkCreateImageView(
            device, 
            &viewInfo, 
            sj::g_vkAllocationFns, &imageView);
        SJ_ASSERT(res == VK_SUCCESS, "Failed to create texture image view");

        return imageView;
    }

    VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice)
    {
        std::array depthFormats = {VK_FORMAT_D32_SFLOAT,
                                   VK_FORMAT_D32_SFLOAT_S8_UINT,
                                   VK_FORMAT_D24_UNORM_S8_UINT};

        VkFormat depthFormat = FindSupportedFormat(physicalDevice,
                                                   depthFormats,
                                                   VK_IMAGE_TILING_OPTIMAL,
                                                   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        return depthFormat;
    }

    VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice,
                                 std::span<VkFormat> candidates,
                                 VkImageTiling tiling,
                                 VkFormatFeatureFlags features)
    {
        for(VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if(tiling == VK_IMAGE_TILING_LINEAR &&
               (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if(tiling == VK_IMAGE_TILING_OPTIMAL &&
                    (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        SJ_ASSERT(false, "Failed to acquire supported VkFormat with given arguments");
        return VK_FORMAT_UNDEFINED;
    }
}