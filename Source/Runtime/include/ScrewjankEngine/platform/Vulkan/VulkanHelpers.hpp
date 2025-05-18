#pragma once

// Screwjank Headers
#include <ScrewjankShared/DataDefinitions/Assets/Model.hpp>

// Library Headers
#include <vulkan/vulkan.h>

// STD Headers
#include <span>
#include <array>

import sj.engine.system.memory;

namespace sj
{
    inline constexpr VkAllocationCallbacks* g_vkAllocationFns = nullptr;

    [[nodiscard]]
    uint32_t FindMemoryType(VkPhysicalDevice physicalDevice,
                            uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);

    void CreateImage(VkDevice logicalDevice,
                     VkPhysicalDevice physicalDevice,
                     uint32_t width,
                     uint32_t height,
                     VkFormat format,
                     VkImageTiling tiling,
                     VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties,
                     VkImage& image,
                     VkDeviceMemory& imageMemory);

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

    
    inline VkVertexInputBindingDescription GetVertexBindingDescription()
    {
        VkVertexInputBindingDescription desc {};

        desc.binding = 0;
        desc.stride = sizeof(Vertex);
        desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return desc;
    }

    inline std::array<VkVertexInputAttributeDescription, 3> GetVertexAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, uv);

        return attributeDescriptions;
    }
    
}