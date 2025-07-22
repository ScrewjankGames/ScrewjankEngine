module;

#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankEngine/components/ComponentMacros.hpp>

#include <vulkan/vulkan.h>

#include <fstream>

export module sj.engine.components.TextureComponent;
import sj.engine.framework.ecs;
import sj.datadefs;
import sj.std.math;

export namespace sj
{
    class TextureComponent
    {
    public:
        TextureComponent(const ECSRegistry& registry, const TextureChunk& chunk) 
        {
            std::ifstream textureStream;
            textureStream.open(chunk.path.c_str());
            // GameObjectId parentGoId = registry.FindGameObject(data.parentId);
            SJ_ASSERT(textureStream.is_open(), "Failed to open texture stream");
        }

        SJ_COMPONENT(TextureComponent, TextureChunk);

        VkImage m_TextureImage;
        VkDeviceMemory m_TextureImageMemory;
        VkImageView m_TextureImageView;
        VkSampler m_TextureSampler;
    };
} // namespace sj