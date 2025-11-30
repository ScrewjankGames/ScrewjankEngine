module;

#include <ScrewjankStd/Assert.hpp>

#include <vk_mem_alloc.h>

#include <fstream>

export module sj.engine.rendering.vk.TextureResource;
import sj.datadefs.assets.Texture;
import sj.engine.rendering.vk.Buffer;
import sj.engine.rendering.vk.ImageUtils;
import sj.engine.rendering.vk.ImmediateCommandContext;
import sj.engine.rendering.vk.Primitives;
import sj.engine.rendering.vk.RenderDevice;

import vulkan_hpp;

export namespace sj::vulkan
{
class TextureResource
{
public:
    TextureResource() = default;
    TextureResource(const char* path,
                    sj::vulkan::RenderDevice& device,
                    ImmediateCommandContext& ctx)
    {
        std::ifstream textureFile;
        textureFile.open("Data/Engine/viking_room.sj_tex", std::ios::in | std::ios::binary);
        TextureHeader header;
        textureFile.read(reinterpret_cast<char*>(&header), sizeof(header));

        VkDeviceSize imageSizeBytes = header.width * header.height * 4;
        sj::vulkan::BufferResource stagingBuffer =
            sj::vulkan::MakeStagingBuffer(device.mAllocator, imageSizeBytes);

        // Read texture data straight into GPU memory
        textureFile.read(reinterpret_cast<char*>(stagingBuffer.GetMappedMemory()), imageSizeBytes);
        textureFile.close();

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocCreateInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vk::Extent3D imageExtent = {static_cast<uint32_t>(header.width),
                                    static_cast<uint32_t>(header.height),
                                    1};

        mImageResource = sj::vulkan::ImageResource(device.mAllocator,
                                                   allocCreateInfo,
                                                   imageExtent,
                                                   vk::Format::eR8G8B8A8Srgb,
                                                   vk::ImageUsageFlagBits::eTransferDst |
                                                       vk::ImageUsageFlagBits::eSampled,
                                                   vk::ImageTiling::eOptimal);

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

        stagingBuffer.DeInit(device.mAllocator);

        mImageView =
            mImageResource.MakeImageView(device.mLogicalDevice, vk::ImageAspectFlagBits::eColor);
        VkPhysicalDeviceProperties properties {};
        vkGetPhysicalDeviceProperties(*device.mPhysicalDevice, &properties);

        vk::SamplerCreateInfo samplerInfo({},
                                          vk::Filter::eLinear,
                                          vk::Filter::eLinear,
                                          vk::SamplerMipmapMode::eLinear,
                                          vk::SamplerAddressMode::eRepeat,
                                          vk::SamplerAddressMode::eRepeat,
                                          vk::SamplerAddressMode::eRepeat,
                                          {},
                                          true,
                                          properties.limits.maxSamplerAnisotropy,
                                          false,
                                          vk::CompareOp::eAlways,
                                          {},
                                          {},
                                          vk::BorderColor::eIntOpaqueBlack,
                                          false);

        mTextureSampler =
            vk::raii::Sampler(device.mLogicalDevice, samplerInfo, g_vkAllocationCallbacks);

        return;
    }

    vk::ImageView GetImageView()
    {
        return mImageView;
    }

    vk::Sampler GetSampler()
    {
        return mTextureSampler;
    }

private:
    sj::vulkan::ImageResource mImageResource;
    vk::raii::ImageView mImageView {nullptr};
    vk::raii::Sampler mTextureSampler {nullptr};
};
} // namespace sj::vulkan