#pragma once

// Library Headers
#include <vulkan/vulkan.h>

namespace sj
{
    class VulkanPipeline
    {
    public:
        VulkanPipeline();
        ~VulkanPipeline();

    private:
        VkShaderModule LoadShaderModule(const char* path);
    };
} // namespace sj
