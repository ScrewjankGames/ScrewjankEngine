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
    VulkanPipeline::VulkanPipeline(VkDevice device,
                                   const char* vertexShaderPath,
                                   const char* fragmentShaderPath)
    {
        VkShaderModule vertexShaderModule = LoadShaderModule(device, vertexShaderPath);
        VkShaderModule fragmentShaderModule = LoadShaderModule(device, fragmentShaderPath);

        vkDestroyShaderModule(device, fragmentShaderModule, nullptr);
        vkDestroyShaderModule(device, vertexShaderModule, nullptr);
    }

    VulkanPipeline::~VulkanPipeline()
    {
    }

    VkShaderModule VulkanPipeline::LoadShaderModule(VkDevice device, const char* path)
    {
        HeapZoneScope rendererWorkBuffer(Renderer::WorkBuffer());
        File shader;
        shader.Open(path, File::OpenMode::kRead);

        uint64_t shaderBufferSize = shader.Size();
        void* shaderBuffer = rendererWorkBuffer->Allocate(shaderBufferSize);
        shader.Read(shaderBuffer, shaderBufferSize);

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.codeSize = shaderBufferSize;
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderBuffer);

        VkShaderModule shaderModule;
        VkResult res = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
        return shaderModule;
    }

} // namespace sj