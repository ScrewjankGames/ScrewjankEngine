// Parent Include
#include <ScrewjankEngine/platform/Vulkan/VulkanPipeline.hpp>

// Library Headers
#include <vulkan/vulkan.h>

// Engine Headers
#include <ScrewjankEngine/rendering/Renderer.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanSwapChain.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanHelpers.hpp>
#include <ScrewjankEngine/system/memory/Memory.hpp>
// Shared Headers
#include <ScrewjankShared/io/File.hpp>

namespace sj
{
    void VulkanPipeline::Init(VkDevice device,
                              VkExtent2D imageExtent,
                              VkRenderPass renderPass,
                              VkDescriptorSetLayout descriptorSetLayout,
                              const char* vertexShaderPath,
                              const char* fragmentShaderPath)
    {
        m_Device = device;

        VkShaderModule vertexShaderModule = LoadShaderModule(vertexShaderPath);
        VkShaderModule fragmentShaderModule = LoadShaderModule(fragmentShaderPath);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertexShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragmentShaderModule;
        fragShaderStageInfo.pName = "main";
        
        static_vector<VkPipelineShaderStageCreateInfo, 2> shaderStages = {vertShaderStageInfo, fragShaderStageInfo};
        
        auto bindingDescription = GetVertexBindingDescription();
        auto attributeDescriptions = GetVertexAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional
        vertexInputInfo.vertexAttributeDescriptionCount =
            static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional

        VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport {};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = (float)imageExtent.width;
        viewport.height = (float)imageExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor {};
        scissor.offset = {0, 0};
        scissor.extent = imageExtent;

        std::array<VkDynamicState, 2> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                                  VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamicState {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineViewportStateCreateInfo viewportState {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        VkPipelineMultisampleStateCreateInfo multisampling {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        VkPipelineColorBlendAttachmentState colorBlendAttachment {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional
        

        VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        {
            VkResult res =
                vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout);
            SJ_ASSERT(res == VK_SUCCESS, "failed to create pipeline layout!");
        }


        VkGraphicsPipelineCreateInfo pipelineInfo {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        
        // Shader stages
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages.data();
        
        // Fixed function stuff
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr; // Optional
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;

        // Pipeline layout
        pipelineInfo.layout = m_PipelineLayout;
        
        // Render Pass
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;

        // Stuff for derivative pipelines
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        VkPipelineDepthStencilStateCreateInfo depthStencil {};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional
        pipelineInfo.pDepthStencilState = &depthStencil;

        {
            VkResult res = vkCreateGraphicsPipelines(m_Device,
                                                     VK_NULL_HANDLE,
                                                     1,
                                                     &pipelineInfo,
                                                     nullptr,
                                                     &m_Pipeline);

            SJ_ASSERT(res == VK_SUCCESS, "Failed to create graphics pipeline");
        }


        vkDestroyShaderModule(m_Device, fragmentShaderModule, nullptr);
        vkDestroyShaderModule(m_Device, vertexShaderModule, nullptr);
    }

    void VulkanPipeline::DeInit()
    {
        vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
        vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
    }

    VkPipeline VulkanPipeline::GetPipeline()
    {
        return m_Pipeline;
    }

    VkPipelineLayout VulkanPipeline::GetLayout()
    {
        return m_PipelineLayout;
    }

    VkShaderModule VulkanPipeline::LoadShaderModule(const char* path)
    {
        MemSpaceScope rendererWorkBuffer(Renderer::WorkBuffer());

        File shader;
        shader.Open(path, File::OpenMode::kReadBinary);

        uint64_t shaderBufferSize = shader.Size();
        void* shaderBuffer = rendererWorkBuffer->Allocate(shaderBufferSize);
        shader.Read(shaderBuffer, shaderBufferSize);

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shaderBufferSize;
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderBuffer);

        VkShaderModule shaderModule;
        VkResult res = vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to load shader module- check the log");

        shader.Close();
        rendererWorkBuffer->Free(shaderBuffer);

        return shaderModule;
    }

} // namespace sj