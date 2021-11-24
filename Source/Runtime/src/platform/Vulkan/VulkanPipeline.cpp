// Parent Include
#include <ScrewjankEngine/platform/Vulkan/VulkanPipeline.hpp>

// Library Headers
#include <vulkan/vulkan.h>

// Engine Headers
#include <ScrewjankEngine/rendering/Renderer.hpp>
#include <ScrewjankEngine/system/Memory.hpp>
#include <ScrewjankEngine/system/filesystem/File.hpp>

namespace sj
{
    VulkanPipeline::VulkanPipeline()
    {
    }

    VulkanPipeline::~VulkanPipeline()
    {
    }

    VkShaderModule VulkanPipeline::LoadShaderModule(const char* path)
    {
        ScopedHeapZone rendererWorkBuffer(Renderer::WorkBuffer());
        File shader;
        shader.Open(path, File::OpenMode::kRead);

        uint64_t shaderBufferSize = shader.Size();
        void* shaderBuffer = rendererWorkBuffer->Allocate(shaderBufferSize);
        shader.Read(shaderBuffer, shaderBufferSize);

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.codeSize = shaderBufferSize;
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderBuffer);

        VkShaderModule shaderModule;
        //VkResult res = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
        return shaderModule;
    }

} // namespace sj