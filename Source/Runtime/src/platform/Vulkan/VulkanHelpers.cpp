// Parent include
#include <ScrewjankEngine/platform/Vulkan/VulkanHelpers.hpp>

// Screwjank Headers
#include <ScrewjankShared/utils/Assert.hpp>

//STD Includes
#include <array>

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
                     const VkImageCreateInfo& imageInfo,
                     VkImage& image,
                     VkDeviceMemory& imageMem)
    {
        VkResult res = vkCreateImage(logicalDevice, &imageInfo, nullptr, &image);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to create dummy texture image");

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice,
                                                   memRequirements.memoryTypeBits,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        res = vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &imageMem);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to allocate dummy texture memory on GPU");

        vkBindImageMemory(logicalDevice, image, imageMem, 0);
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
        VkResult res = vkCreateImageView(device, &viewInfo, nullptr, &imageView);
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