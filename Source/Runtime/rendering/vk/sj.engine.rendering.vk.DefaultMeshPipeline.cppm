module;

#include <ScrewjankStd/Assert.hpp>

#include <vulkan/vulkan.h>

#include <array>
#include <ranges>

export module sj.engine.rendering.vk.DefaultMeshPipeline;
import sj.std.math;
import sj.engine.system.threading.ThreadContext;
import sj.engine.rendering.vk.Primitives;
import sj.datadefs.assets.Mesh;
import sj.engine.rendering.vk.Pipeline;

export namespace sj::vulkan
{
    struct MeshDrawPushConstants
    {
        Mat44 meshToWorldMatrix = Mat44(kIdentityTag);
    };

    inline VkVertexInputBindingDescription GetVertexBindingDescription()
    {
        VkVertexInputBindingDescription desc {};

        desc.binding = 0;
        desc.stride = sizeof(MeshVertex);
        desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return desc;
    }

    inline std::ranges::range auto GetVertexAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(MeshVertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(MeshVertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(MeshVertex, uv);

        return attributeDescriptions;
    }

    PipelineResource MakeDefaultMeshPipeline(VkDevice device,
                                             VkDescriptorSetLayout descriptorSetLayout,
                                             VkFormat colorAttachmentFormat,
                                             VkFormat depthAttachmentFormat,
                                             const char* vertexShaderPath,
                                             const char* fragmentShaderPath)
    {

        VkShaderModule vertexShaderModule = LoadShaderModule(device, vertexShaderPath);
        VkShaderModule fragmentShaderModule = LoadShaderModule(device, fragmentShaderPath);

        scratchpad_scope scope = ThreadContext::GetScratchpad();
        PipelineBuilder builder(&scope.get_allocator());

        builder.SetShaders(vertexShaderModule, fragmentShaderModule);

        auto bindingDescription = GetVertexBindingDescription();
        auto attributeDescriptions = GetVertexAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional
        vertexInputInfo.vertexAttributeDescriptionCount =
            static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional

        builder.SetVertexInputState(vertexInputInfo);

        builder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

        builder.SetPolygonMode(VK_POLYGON_MODE_FILL);
        builder.SetCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        builder.SetMultiSamplingNone();
        builder.DisableBlending();
        builder.EnableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);
        builder.SetDepthFormat(depthAttachmentFormat);

        builder.SetColorAttachmentFormat(colorAttachmentFormat);

        {
            VkPipelineLayout layout = {};
            VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
            pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
            pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

            VkResult res = vkCreatePipelineLayout(device,
                                                  &pipelineLayoutInfo,
                                                  sj::g_vkAllocationFns,
                                                  &layout);
            SJ_ASSERT(res == VK_SUCCESS, "failed to create pipeline layout!");
            builder.SetPipelineLayout(layout);
        }

        PipelineResource pipeline = builder.BuildPipeline(device);
        vkDestroyShaderModule(device, fragmentShaderModule, sj::g_vkAllocationFns);
        vkDestroyShaderModule(device, vertexShaderModule, sj::g_vkAllocationFns);

        return pipeline;
    }

} // namespace sj::vulkan