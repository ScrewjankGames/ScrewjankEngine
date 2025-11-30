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
import vulkan_hpp;

export namespace sj::vulkan
{
struct MeshDrawPushConstants
{
    Mat44 meshToWorldMatrix = Mat44(kIdentityTag);
};

inline vk::VertexInputBindingDescription GetVertexBindingDescription()
{
    vk::VertexInputBindingDescription desc {};

    desc.binding = 0;
    desc.stride = sizeof(MeshVertex);
    desc.inputRate = vk::VertexInputRate::eVertex;

    return desc;
}

inline std::ranges::range auto GetVertexAttributeDescriptions()
{
    std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions {};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
    attributeDescriptions[0].offset = offsetof(MeshVertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
    attributeDescriptions[1].offset = offsetof(MeshVertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
    attributeDescriptions[2].offset = offsetof(MeshVertex, uv);

    return attributeDescriptions;
}

PipelineResource MakeDefaultMeshPipeline(vk::raii::Device& device,
                                         vk::DescriptorSetLayout descriptorSetLayout,
                                         vk::Format colorAttachmentFormat,
                                         vk::Format depthAttachmentFormat,
                                         const char* vertexShaderPath,
                                         const char* fragmentShaderPath)
{

    vk::raii::ShaderModule vertexShaderModule = LoadShaderModule(device, vertexShaderPath);
    vk::raii::ShaderModule fragmentShaderModule = LoadShaderModule(device, fragmentShaderPath);

    scratchpad_scope scope = ThreadContext::GetScratchpad();
    PipelineBuilder builder(&scope.get_allocator());

    builder.SetShaders(vertexShaderModule, fragmentShaderModule);

    auto bindingDescription = GetVertexBindingDescription();
    auto attributeDescriptions = GetVertexAttributeDescriptions();

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional

    builder.SetVertexInputState(vertexInputInfo);

    builder.SetInputTopology(vk::PrimitiveTopology::eTriangleList);

    builder.SetPolygonMode(vk::PolygonMode::eFill);
    builder.SetCullMode(vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise);
    builder.SetMultiSamplingNone();
    builder.DisableBlending();
    builder.EnableDepthTest(true, vk::CompareOp::eGreaterOrEqual);
    builder.SetDepthFormat(depthAttachmentFormat);

    builder.SetColorAttachmentFormat(colorAttachmentFormat);

    {
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo {};
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        vk::raii::PipelineLayout layout(device, pipelineLayoutInfo, g_vkAllocationCallbacks);
        builder.SetPipelineLayout(std::move(layout));
    }

    return builder.BuildPipeline(device);
}

} // namespace sj::vulkan