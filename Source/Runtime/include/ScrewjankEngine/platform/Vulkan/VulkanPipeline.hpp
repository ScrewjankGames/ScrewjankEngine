#pragma once

// Library Headers
#include <vulkan/vulkan.h>

namespace sj
{
    class VulkanPipeline
    {
    public:
        VulkanPipeline(VkDevice device, const char* vertexShaderPath, const char* fragmentShaderPath);
        ~VulkanPipeline();

    private:
        VkShaderModule LoadShaderModule(VkDevice device, const char* path);
    };
} // namespace sj
