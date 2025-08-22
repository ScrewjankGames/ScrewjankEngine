module;

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

export module sj.engine.rendering.vk.Primitives;

export namespace sj::vk
{
    struct AllocatedImage
    {
        VkImage image;
        VkImageView imageView;
        VmaAllocation allocation;
        VkExtent3D imageExtent;
        VkFormat imageFormat;
    };
} // namespace sj::vk