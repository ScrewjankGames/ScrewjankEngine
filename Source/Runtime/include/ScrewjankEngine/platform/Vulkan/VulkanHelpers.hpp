#pragma once

// Screwjank Headers


// Library Headers
#include <vulkan/vulkan.h>

// STD Headers
#include <cstdint>
#include <span>

namespace sj
{
    [[nodiscard]]
    uint32_t FindMemoryType(VkPhysicalDevice physicalDevice,
                            uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);

    void CreateImage(VkDevice logicalDevice,
                     VkPhysicalDevice physicalDevice,
                     const VkImageCreateInfo& imageInfo,
                     VkImage& image,
                     VkDeviceMemory& imageMem);
    [[nodiscard]]
    VkImageView CreateImageView(VkDevice device,
                                VkImage image,
                                VkFormat format,
                                VkImageAspectFlags aspectFlags);
    
    [[nodiscard]]
    VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice);

    [[nodiscard]]
    VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice, 
                                 std::span<VkFormat> candidates,
                                 VkImageTiling tiling,
                                 VkFormatFeatureFlags features);
    
}