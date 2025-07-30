module;

#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankEngine/components/ComponentMacros.hpp>

#include <vulkan/vulkan_core.h>

#include <fstream>

export module sj.engine.ecs.components.TextureComponent;
import sj.engine.framework.Engine;

import sj.engine.rendering.Renderer;
import sj.engine.rendering.vk.RenderDevice;
import sj.engine.rendering.vk.Buffer;


import sj.engine.ecs;

import sj.datadefs.DataChunk;
import sj.datadefs.components.TextureChunk;
import sj.datadefs.assets.Texture;

import sj.std.math;

export namespace sj
{
    class TextureComponent
    {
    public:
        TextureComponent(const TextureChunk& chunk)
        {
            std::ifstream textureStream;
            textureStream.open(chunk.path.c_str(), std::ios::in | std::ios::binary);
            SJ_ASSERT(textureStream.is_open(), "Failed to open texture stream");
            
            TextureHeader header;
            textureStream.read(reinterpret_cast<char*>(&header), sizeof(header));

            Renderer* renderer = Engine::GetRenderer();
            const sj::vk::RenderDevice& renderDevice = *renderer->GetRenderDevice();

            VkDeviceSize imageSize = header.width * header.height * 4;
            VkBuffer stagingBuffer {};
            VkDeviceMemory stagingBufferMemory {};

            sj::vk::CreateBuffer(renderDevice,
                         imageSize,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer,
                         stagingBufferMemory);

            void* data = nullptr;
            vkMapMemory(renderDevice.GetLogicalDevice(),
                        stagingBufferMemory,
                        0,
                        imageSize,
                        0,
                        &data);

            // // Read texture data straight into GPU memory
            textureStream.read(reinterpret_cast<char*>(data), imageSize);
            vkUnmapMemory(renderDevice.GetLogicalDevice(), stagingBufferMemory);

            textureStream.close();
            // CreateImage(renderDevice,
            //             header.width,
            //             header.height,
            //             VK_FORMAT_R8G8B8A8_SRGB,
            //             VK_IMAGE_TILING_OPTIMAL,
            //             VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            //             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            //             m_TextureImage,
            //             m_TextureImageMemory);

            // TransitionImageLayout(m_dummyTextureImage,
            //                       VK_IMAGE_LAYOUT_UNDEFINED,
            //                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            // CopyBufferToImage(stagingBuffer, m_dummyTextureImage, header.width, header.height);

            // TransitionImageLayout(m_dummyTextureImage,
            //                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            //                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            // vkDestroyBuffer(m_renderDevice.GetLogicalDevice(),
            //                 stagingBuffer,
            //                 sj::g_vkAllocationFns);
            // vkFreeMemory(m_renderDevice.GetLogicalDevice(),
            //              stagingBufferMemory,
            //              sj::g_vkAllocationFns);

            return;
        }

        SJ_COMPONENT(TextureComponent, TextureChunk);

        VkImage m_TextureImage = VK_NULL_HANDLE;
        VkDeviceMemory m_TextureImageMemory = VK_NULL_HANDLE;
        VkImageView m_TextureImageView = VK_NULL_HANDLE;
        VkSampler m_TextureSampler = VK_NULL_HANDLE;
    };
} // namespace sj