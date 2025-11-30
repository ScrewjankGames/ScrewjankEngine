module;
#include <vulkan/vulkan.h>
#include <optional>

export module sj.engine.rendering.vk.Helpers;
import vulkan_hpp;

export namespace sj::vulkan
{

VkCommandBufferBeginInfo MakeCommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0)
{
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = nullptr;

    info.pInheritanceInfo = nullptr;
    info.flags = flags;
    return info;
}

VkImageCreateInfo
MakeImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent)
{
    VkImageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = nullptr;

    info.imageType = VK_IMAGE_TYPE_2D;

    info.format = format;
    info.extent = extent;

    info.mipLevels = 1;
    info.arrayLayers = 1;

    info.samples = VK_SAMPLE_COUNT_1_BIT;

    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = usageFlags;

    return info;
}

vk::ImageViewCreateInfo
MakeImageViewCreateInfo(vk::Format format, vk::Image image, vk::ImageAspectFlags aspectFlags)
{
    return vk::ImageViewCreateInfo({},
                                   image,
                                   vk::ImageViewType::e2D,
                                   format,
                                   {},
                                   vk::ImageSubresourceRange(aspectFlags, 0, 1, 0, 1));
}

vk::RenderingAttachmentInfo
MakeAttachmentInfo(vk::ImageView view,
                   std::optional<vk::ClearValue> opt_clearValue,
                   vk::ImageLayout layout = vk::ImageLayout::eColorAttachmentOptimal)
{
    vk::RenderingAttachmentInfo colorAttachment;
    colorAttachment.setImageView(view);
    colorAttachment.setImageLayout(layout);
    colorAttachment.setLoadOp(opt_clearValue ? vk::AttachmentLoadOp::eClear
                                             : vk::AttachmentLoadOp::eLoad);
    colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    colorAttachment.setClearValue(opt_clearValue.value_or({}));

    return colorAttachment;
}

vk::RenderingAttachmentInfo MakeDepthAttachmentInfo(vk::ImageView view, vk::ImageLayout layout)
{
    vk::RenderingAttachmentInfo depthAttachment;
    depthAttachment.setImageView(view);
    depthAttachment.setImageLayout(layout);
    depthAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
    depthAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    depthAttachment.setClearValue(vk::ClearDepthStencilValue(0.0f));

    return depthAttachment;
}

VkRenderingInfo MakeRenderingInfo(VkExtent2D renderExtent,
                                  VkRenderingAttachmentInfo* colorAttachment,
                                  VkRenderingAttachmentInfo* depthAttachment)
{
    VkRenderingInfo renderInfo {};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderInfo.pNext = nullptr;

    renderInfo.renderArea = VkRect2D {VkOffset2D {0, 0}, renderExtent};
    renderInfo.layerCount = 1;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachments = colorAttachment;
    renderInfo.pDepthAttachment = depthAttachment;
    renderInfo.pStencilAttachment = nullptr;

    return renderInfo;
}

} // namespace sj::vulkan