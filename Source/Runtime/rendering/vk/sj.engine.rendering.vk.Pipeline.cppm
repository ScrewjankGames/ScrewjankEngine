module;

// Library Headers
#include <vulkan/vulkan_core.h>

// ScrewjankStd Headers
#include <ScrewjankStd/Assert.hpp>

// STD Includes
#include <array>
#include <fstream>
#include <memory_resource>

export module sj.engine.rendering.vk.Pipeline;
import sj.engine.system.threading.ThreadContext;
import sj.engine.rendering.vk.Utils;
import sj.engine.rendering.vk.Primitives;
import sj.std.containers.vector;

import vulkan_hpp;

export namespace sj::vulkan
{
vk::raii::ShaderModule LoadShaderModule(vk::raii::Device& device, const char* path)
{
    scratchpad_scope scratchpad = ThreadContext::GetScratchpad();
    std::ifstream shader;
    shader.open(path, std::ios::in | std::ios::binary | std::ios::ate);
    SJ_ASSERT(shader.is_open(), "Failed to open compiled shader {}", path);
    size_t shaderBufferSize = shader.tellg();
    shader.seekg(0);

    char* shaderBuffer =
        reinterpret_cast<char*>(scratchpad.get_allocator().allocate(shaderBufferSize));
    shader.read(shaderBuffer, shaderBufferSize);

    vk::ShaderModuleCreateInfo createInfo({},
                                          shaderBufferSize,
                                          reinterpret_cast<const uint32_t*>(shaderBuffer));
    return vk::raii::ShaderModule(device, createInfo, g_vkAllocationCallbacks);
}

struct PipelineResource
{
    vk::raii::Pipeline pipeline {nullptr};
    vk::raii::PipelineLayout layout {nullptr};
};

class PipelineBuilder
{
public:
    PipelineBuilder(std::pmr::memory_resource* resource) : m_shaderStages(resource)
    {
    }

    void SetShaders(const vk::raii::ShaderModule& vertexShader,
                    const vk::raii::ShaderModule& fragmentShader)
    {
        auto MakeShaderStageCreateInfoFn =
            [](vk::ShaderModule module,
               vk::ShaderStageFlagBits bits) -> vk::PipelineShaderStageCreateInfo {
            return vk::PipelineShaderStageCreateInfo({}, bits, module, "main");
        };

        m_shaderStages.emplace_back(
            MakeShaderStageCreateInfoFn(vertexShader, vk::ShaderStageFlagBits::eVertex));
        m_shaderStages.emplace_back(
            MakeShaderStageCreateInfoFn(fragmentShader, vk::ShaderStageFlagBits::eFragment));
    }

    void SetVertexInputState(vk::PipelineVertexInputStateCreateInfo vertexInfo)
    {
        m_vertextInput = vertexInfo;
    }

    void SetInputTopology(vk::PrimitiveTopology topo)
    {
        m_inputAssembly.topology = topo;
        m_inputAssembly.primitiveRestartEnable = false;
    }

    void SetPolygonMode(vk::PolygonMode mode, float lineWidth = 1.0f)
    {
        m_rasterizer.polygonMode = mode;
        m_rasterizer.lineWidth = lineWidth;
    }

    void SetCullMode(vk::CullModeFlags cullModeFlags, vk::FrontFace frontFace)
    {
        m_rasterizer.cullMode = cullModeFlags;
        m_rasterizer.frontFace = frontFace;
    }

    void SetMultiSamplingNone()
    {
        m_multisampling.sampleShadingEnable = false;
        // multisampling defaulted to no multisampling (1 sample per pixel)
        m_multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
        m_multisampling.minSampleShading = 1.0f;
        m_multisampling.pSampleMask = nullptr;
        // no alpha to coverage either
        m_multisampling.alphaToCoverageEnable = false;
        m_multisampling.alphaToOneEnable = false;
    }

    void DisableBlending()
    {
        // default write mask
        m_colorBlendAttachment.colorWriteMask =
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

        // no blending
        m_colorBlendAttachment.blendEnable = false;
    }

    void SetColorAttachmentFormat(vk::Format format)
    {
        m_colorAttachmentformat = format;

        // connect the format to the renderInfo  structure
        m_renderInfo.colorAttachmentCount = 1;
        m_renderInfo.pColorAttachmentFormats = &m_colorAttachmentformat;
    }

    void SetDepthFormat(vk::Format format)
    {
        m_renderInfo.depthAttachmentFormat = format;
    }

    void DisableDepthTest()
    {
        m_depthStencil.depthTestEnable = false;
        m_depthStencil.depthWriteEnable = false;
        m_depthStencil.depthCompareOp = vk::CompareOp::eNever;
        m_depthStencil.depthBoundsTestEnable = false;
        m_depthStencil.stencilTestEnable = false;
        m_depthStencil.setFront({});
        m_depthStencil.setBack({});
        m_depthStencil.minDepthBounds = 0.f;
        m_depthStencil.maxDepthBounds = 1.f;
    }

    void EnableDepthTest(bool depthWriteEnable, vk::CompareOp op)
    {
        m_depthStencil.depthWriteEnable = depthWriteEnable;
        m_depthStencil.depthCompareOp = op;

        m_depthStencil.depthTestEnable = VK_TRUE;
        m_depthStencil.depthBoundsTestEnable = false;
        m_depthStencil.stencilTestEnable = false;

        m_depthStencil.minDepthBounds = 0.f;
        m_depthStencil.maxDepthBounds = 1.f;
    }

    void SetPipelineLayout(vk::raii::PipelineLayout&& layout)
    {
        m_pipelineLayout = std::move(layout);
    }

    [[nodiscard]] PipelineResource BuildPipeline(vk::raii::Device& device)
    {
        // Just filling viewport and scissor count because we're using dynamic viewport state
        vk::PipelineViewportStateCreateInfo viewportState;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        // setup dummy color blending. We arent using transparent objects yet
        // the blending is just "no blend", but we do write to the color attachment
        vk::PipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.logicOpEnable = false;
        colorBlending.logicOp = vk::LogicOp::eCopy;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &m_colorBlendAttachment;

        vk::GraphicsPipelineCreateInfo pipelineInfo;

        // connect the renderInfo to the pNext extension mechanism
        pipelineInfo.pNext = &m_renderInfo;

        pipelineInfo.stageCount = (uint32_t)m_shaderStages.size();
        pipelineInfo.pStages = m_shaderStages.data();
        pipelineInfo.pVertexInputState = &m_vertextInput;
        pipelineInfo.pInputAssemblyState = &m_inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &m_rasterizer;
        pipelineInfo.pMultisampleState = &m_multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDepthStencilState = &m_depthStencil;
        pipelineInfo.layout = m_pipelineLayout;

        std::array dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

        vk::PipelineDynamicStateCreateInfo dynamicInfo({}, dynamicStates);
        pipelineInfo.pDynamicState = &dynamicInfo;

        vk::raii::Pipeline newPipeline(device, nullptr, pipelineInfo, g_vkAllocationCallbacks);

        return PipelineResource {.pipeline = std::move(newPipeline),
                                 .layout = std::move(m_pipelineLayout)};
    }

private:
    sj::dynamic_vector<vk::PipelineShaderStageCreateInfo> m_shaderStages;

    vk::PipelineInputAssemblyStateCreateInfo m_inputAssembly {};
    vk::PipelineVertexInputStateCreateInfo m_vertextInput {};
    vk::PipelineRasterizationStateCreateInfo m_rasterizer {};
    vk::PipelineColorBlendAttachmentState m_colorBlendAttachment {};
    vk::PipelineMultisampleStateCreateInfo m_multisampling {};

    vk::PipelineDepthStencilStateCreateInfo m_depthStencil {};
    vk::PipelineRenderingCreateInfo m_renderInfo {};
    vk::Format m_colorAttachmentformat {};

    vk::raii::PipelineLayout m_pipelineLayout {nullptr};
};
} // namespace sj::vulkan