module;

#include <vulkan/vulkan_core.h>

export module sj.engine.rendering.resources.TextureResource;

export namespace sj
{
    struct TextureResource
    {
        VkImage m_TextureImage = VK_NULL_HANDLE;
        VkDeviceMemory m_TextureImageMemory = VK_NULL_HANDLE;
        VkImageView m_TextureImageView = VK_NULL_HANDLE;
        VkSampler m_TextureSampler = VK_NULL_HANDLE;
    };
}