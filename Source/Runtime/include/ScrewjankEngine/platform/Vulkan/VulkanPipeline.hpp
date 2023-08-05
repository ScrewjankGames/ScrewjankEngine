#pragma once

// Library Headers
#include <vulkan/vulkan.h>

// SJ Headers
#include <ScrewjankEngine/containers/StaticVector.hpp>

namespace sj
{
    class VulkanPipeline
    {
    public:
        VulkanPipeline() = default;
        ~VulkanPipeline() = default;

        void Init(VkDevice device,
                  VkExtent2D imageExtent,
                  VkRenderPass renderPass,
                  const char* vertexShaderPath,
                  const char* fragmentShaderPath);

        void DeInit();

    private:
        VkShaderModule LoadShaderModule(const char* path);
        VkDevice m_Device;
        VkPipelineLayout m_PipelineLayout;
        VkPipeline m_Pipeline;
    };
} // namespace sj
