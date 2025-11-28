module;

#include <ScrewjankStd/Assert.hpp>

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

#include <fstream>

export module sj.engine.rendering.vk.TextureResource;
import sj.datadefs.assets.Texture;
import sj.engine.rendering.vk.Buffer;
import sj.engine.rendering.vk.ImageUtils;
import sj.engine.rendering.vk.ImmediateCommandContext;
import sj.engine.rendering.vk.Primitives;
import sj.engine.rendering.vk.RenderDevice;

export namespace sj::vulkan
{
class TextureResource
{
public:
    void Init(const char* path, sj::vulkan::RenderDevice& device, ImmediateCommandContext& ctx)
    {
        std::ifstream textureFile;
        textureFile.open("Data/Engine/viking_room.sj_tex", std::ios::in | std::ios::binary);
        TextureHeader header;
        textureFile.read(reinterpret_cast<char*>(&header), sizeof(header));

        VkDeviceSize imageSizeBytes = header.width * header.height * 4;
        sj::vulkan::BufferResource stagingBuffer =
            sj::vulkan::MakeStagingBuffer(device.GetAllocator(), imageSizeBytes);

        // Read texture data straight into GPU memory
        textureFile.read(reinterpret_cast<char*>(stagingBuffer.GetMappedMemory()), imageSizeBytes);
        textureFile.close();

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocCreateInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkExtent3D imageExtent = {static_cast<uint32_t>(header.width),
                                  static_cast<uint32_t>(header.height),
                                  1};

        mImageResource =
            sj::vulkan::ImageResource(device.GetAllocator(),
                                  allocCreateInfo,
                                  imageExtent,
                                  VK_FORMAT_R8G8B8A8_SRGB,
                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                  VK_IMAGE_TILING_OPTIMAL);

        VkImage textureImage = mImageResource.GetImage();
        ctx.ImmediateSubmit([textureImage](VkCommandBuffer cmd) {
            sj::vulkan::TransitionImage(cmd,
                                    textureImage,
                                    VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        });

        ctx.ImmediateSubmit([buf = stagingBuffer.GetBuffer(),
                             img = mImageResource.GetImage(),
                             header](VkCommandBuffer cmd) {
            sj::vulkan::CopyBufferToImage(cmd, buf, img, header.width, header.height);
        });

        ctx.ImmediateSubmit([textureImage](VkCommandBuffer cmd) {
            sj::vulkan::TransitionImage(cmd,
                                    textureImage,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        });

        stagingBuffer.DeInit(device.GetAllocator());

        mImageView =
            mImageResource.MakeImageView(device.GetLogicalDevice(), VK_IMAGE_ASPECT_COLOR_BIT);
        VkPhysicalDeviceProperties properties {};
        vkGetPhysicalDeviceProperties(device.GetPhysicalDevice(), &properties);

        VkSamplerCreateInfo samplerInfo {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        VkResult res = vkCreateSampler(device.GetLogicalDevice(),
                                       &samplerInfo,
                                       sj::g_vkAllocationFns,
                                       &mTextureSampler);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to create image sampler");
        return;
    }

    void DeInit(VkDevice device, VmaAllocator allocator)
    {
        vkDestroySampler(device, mTextureSampler, sj::g_vkAllocationFns);
        vkDestroyImageView(device, mImageView, sj::g_vkAllocationFns);
        mImageResource.DeInit(allocator);
    }

    VkImageView GetImageView()
    {
        return mImageView;
    }

    VkSampler GetSampler()
    {
        return mTextureSampler;
    }

private:
    sj::vulkan::ImageResource mImageResource;
    VkImageView mImageView {};
    VkSampler mTextureSampler {};
};
} // namespace sj::vulkan