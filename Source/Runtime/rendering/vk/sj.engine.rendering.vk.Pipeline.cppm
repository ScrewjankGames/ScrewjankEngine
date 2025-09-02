module;

// Library Headers
#include <vulkan/vulkan.h>

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

export namespace sj::vk
{
    class PipelineBuilder
    {
    public:
        PipelineBuilder(std::pmr::memory_resource* resource) : m_shaderStages(resource)
        {
            Clear();
        }

        void Clear()
        {
            m_inputAssembly = {.sType =
                                   VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
            m_rasterizer = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
            m_colorBlendAttachment = {};
            m_multisampling = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
            m_pipelineLayout = {};
            m_depthStencil = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
            m_renderInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
            m_shaderStages.clear();
        }

        void SetShaders(VkShaderModule vertexShader, VkShaderModule fragmentShader)
        {
            auto MakeShaderStageCreateInfoFn =
                [](VkShaderModule module,
                   VkShaderStageFlagBits bits) -> VkPipelineShaderStageCreateInfo {
                VkPipelineShaderStageCreateInfo shaderInfo {};
                shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shaderInfo.stage = bits;
                shaderInfo.module = module;
                shaderInfo.pName = "main";

                return shaderInfo;
            };

            m_shaderStages.emplace_back(
                MakeShaderStageCreateInfoFn(vertexShader, VK_SHADER_STAGE_VERTEX_BIT));
            m_shaderStages.emplace_back(
                MakeShaderStageCreateInfoFn(fragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT));
        }

        void SetVertexInputState(VkPipelineVertexInputStateCreateInfo vertexInfo)
        {
            m_vertextInput = vertexInfo;
        }

        void SetInputTopology(VkPrimitiveTopology topo)
        {
            m_inputAssembly.topology = topo;
            m_inputAssembly.primitiveRestartEnable = VK_FALSE;
        }

        void SetPolygonMode(VkPolygonMode mode, float lineWidth = 1.0f)
        {
            m_rasterizer.polygonMode = mode;
            m_rasterizer.lineWidth = lineWidth;
        }

        void SetCullMode(VkCullModeFlags cullModeFlags, VkFrontFace frontFace)
        {
            m_rasterizer.cullMode = cullModeFlags;
            m_rasterizer.frontFace = frontFace;
        }

        void SetMultiSamplingNone()
        {
            m_multisampling.sampleShadingEnable = VK_FALSE;
            // multisampling defaulted to no multisampling (1 sample per pixel)
            m_multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            m_multisampling.minSampleShading = 1.0f;
            m_multisampling.pSampleMask = nullptr;
            // no alpha to coverage either
            m_multisampling.alphaToCoverageEnable = VK_FALSE;
            m_multisampling.alphaToOneEnable = VK_FALSE;
        }

        void DisableBlending()
        {
            // default write mask
            m_colorBlendAttachment.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT;

            // no blending
            m_colorBlendAttachment.blendEnable = VK_FALSE;
        }

        void SetColorAttachmentFormat(VkFormat format)
        {
            m_colorAttachmentformat = format;

            // connect the format to the renderInfo  structure
            m_renderInfo.colorAttachmentCount = 1;
            m_renderInfo.pColorAttachmentFormats = &m_colorAttachmentformat;
        }

        void SetDepthFormat(VkFormat format)
        {
            m_renderInfo.depthAttachmentFormat = format;
        }

        void DisableDepthTest()
        {
            m_depthStencil.depthTestEnable = VK_FALSE;
            m_depthStencil.depthWriteEnable = VK_FALSE;
            m_depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
            m_depthStencil.depthBoundsTestEnable = VK_FALSE;
            m_depthStencil.stencilTestEnable = VK_FALSE;
            m_depthStencil.front = {};
            m_depthStencil.back = {};
            m_depthStencil.minDepthBounds = 0.f;
            m_depthStencil.maxDepthBounds = 1.f;
        }

        void EnableDepthTest()
        {
            m_depthStencil.depthTestEnable = VK_TRUE;
            m_depthStencil.depthWriteEnable = VK_TRUE;
            m_depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
            m_depthStencil.depthBoundsTestEnable = VK_FALSE;
            m_depthStencil.minDepthBounds = 0.0f; // Optional
            m_depthStencil.maxDepthBounds = 1.0f; // Optional
            m_depthStencil.stencilTestEnable = VK_FALSE;
            m_depthStencil.front = {}; // Optional
            m_depthStencil.back = {}; // Optional
        }

        void SetPipelineLayout(VkPipelineLayout layout)
        {
            m_pipelineLayout = layout;
        }

        [[nodiscard]] VkPipeline BuildPipeline(VkDevice device) const
        {
            // Just filling viewport and scissor count because we're using dynamic viewport state
            VkPipelineViewportStateCreateInfo viewportState = {};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.pNext = nullptr;

            viewportState.viewportCount = 1;
            viewportState.scissorCount = 1;

            // setup dummy color blending. We arent using transparent objects yet
            // the blending is just "no blend", but we do write to the color attachment
            VkPipelineColorBlendStateCreateInfo colorBlending = {};
            colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlending.pNext = nullptr;

            colorBlending.logicOpEnable = VK_FALSE;
            colorBlending.logicOp = VK_LOGIC_OP_COPY;
            colorBlending.attachmentCount = 1;
            colorBlending.pAttachments = &m_colorBlendAttachment;

            VkGraphicsPipelineCreateInfo pipelineInfo = {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};

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

            std::array dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

            VkPipelineDynamicStateCreateInfo dynamicInfo = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
            dynamicInfo.pDynamicStates = dynamicStates.data();
            dynamicInfo.dynamicStateCount = dynamicStates.size();

            pipelineInfo.pDynamicState = &dynamicInfo;

            [[indeterminate]] VkPipeline newPipeline;
            VkResult status = vkCreateGraphicsPipelines(device,
                                                        VK_NULL_HANDLE,
                                                        1,
                                                        &pipelineInfo,
                                                        sj::g_vkAllocationFns,
                                                        &newPipeline);
            SJ_ASSERT(status == VK_SUCCESS, "Failed to create graphics pipeline!");

            return newPipeline;
        }

    private:
        sj::dynamic_vector<VkPipelineShaderStageCreateInfo> m_shaderStages;

        VkPipelineInputAssemblyStateCreateInfo m_inputAssembly;
        VkPipelineVertexInputStateCreateInfo m_vertextInput;
        VkPipelineRasterizationStateCreateInfo m_rasterizer;
        VkPipelineColorBlendAttachmentState m_colorBlendAttachment;
        VkPipelineMultisampleStateCreateInfo m_multisampling;
        VkPipelineLayout m_pipelineLayout;
        VkPipelineDepthStencilStateCreateInfo m_depthStencil;
        VkPipelineRenderingCreateInfo m_renderInfo;
        VkFormat m_colorAttachmentformat;
    };

    class Pipeline
    {
    public:
        Pipeline() = default;
        ~Pipeline() = default;

        void Init(VkDevice device,
                  VkDescriptorSetLayout descriptorSetLayout,
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
            builder.DisableDepthTest(); // TODO: Re-enable

            {
                VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
                pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                pipelineLayoutInfo.setLayoutCount = 1;
                pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
                pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
                pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

                VkResult res = vkCreatePipelineLayout(device,
                                                      &pipelineLayoutInfo,
                                                      sj::g_vkAllocationFns,
                                                      &m_PipelineLayout);
                SJ_ASSERT(res == VK_SUCCESS, "failed to create pipeline layout!");
            }

            builder.SetPipelineLayout(m_PipelineLayout);
            m_Pipeline = builder.BuildPipeline(device);

            vkDestroyShaderModule(device, fragmentShaderModule, sj::g_vkAllocationFns);
            vkDestroyShaderModule(device, vertexShaderModule, sj::g_vkAllocationFns);
        }

        void DeInit(VkDevice device)
        {
            vkDestroyPipeline(device, m_Pipeline, sj::g_vkAllocationFns);
            vkDestroyPipelineLayout(device, m_PipelineLayout, sj::g_vkAllocationFns);
        }

        VkPipeline GetPipeline()
        {
            return m_Pipeline;
        }

        VkPipelineLayout GetLayout()
        {
            return m_PipelineLayout;
        }

    private:
        VkShaderModule LoadShaderModule(VkDevice device, const char* path)
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

            VkShaderModuleCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = shaderBufferSize;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderBuffer);

            VkShaderModule shaderModule = {};
            VkResult res =
                vkCreateShaderModule(device, &createInfo, sj::g_vkAllocationFns, &shaderModule);

            SJ_ASSERT(res == VK_SUCCESS, "Failed to load shader module- check the log");

            shader.close();

            return shaderModule;
        }

        VkPipelineLayout m_PipelineLayout;
        VkPipeline m_Pipeline;
    };
} // namespace sj::vk