module;

// Screwjank Headers
#include <ScrewjankStd/PlatformDetection.hpp>
#include <ScrewjankStd/Log.hpp>
#include <ScrewjankStd/Assert.hpp>

// Library Headers
#include <vulkan/vulkan_hpp_macros.hpp>
#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

#ifndef SJ_GOLD
    #include <imgui_impl_vulkan.h>
#endif

// STD Headers
#include <array>
#include <cstring>
#include <cstddef>
#include <fstream>
#include <optional>

export module sj.engine.rendering.Renderer;
import sj.engine.core.Program;
import sj.engine.core.Window;
import sj.engine.rendering.vk;
import sj.engine.system.threading.ThreadContext;
import sj.engine.system.memory.MemorySystem;
import sj.datadefs.assets.Texture;
import sj.datadefs.assets.Mesh;
import sj.std.containers.vector;
import sj.std.math;
import sj.std.memory.literals;
import sj.std.memory.resources.free_list_allocator;

import vulkan_hpp;

export namespace sj
{
class Renderer
{
public:
    static free_list_allocator* WorkBuffer()
    {
        static free_list_allocator g_workBufferResource;
        return &g_workBufferResource;
    }

    Renderer() = default;

    void Initialize(auto& program)
    {
        m_display = program.template GetModule<Window>();

        free_list_allocator* workBuffer = WorkBuffer();
        workBuffer->init(4_MiB, *MemorySystem::GetRootMemoryResource());
        MemorySystem::TrackMemoryResource(workBuffer);

#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
        // initialize minimal set of function pointers
        VULKAN_HPP_DEFAULT_DISPATCHER.init();
#endif

        auto&& [vkbInstance, surface, debugMessenger, vkbPhysicalDevice, vkbDevice] =
            Bootstrap(m_display);
        mVkInstance = vk::raii::Instance(mVkContext, vkbInstance);

#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
        // Load the rest of the pointers
        VULKAN_HPP_DEFAULT_DISPATCHER.init(*mVkInstance);
#endif
        mSurface = vk::raii::SurfaceKHR(mVkInstance, surface);

#ifndef SJ_GOLD
        mVkDebugMessenger =
            vk::raii::DebugUtilsMessengerEXT(mVkInstance, vkbInstance.debug_messenger);
#endif

        mRenderDevice.mPhysicalDevice = vk::raii::PhysicalDevice(mVkInstance, vkbPhysicalDevice);
        mRenderDevice.mLogicalDevice =
            vk::raii::Device(mRenderDevice.mPhysicalDevice, vkbDevice.device);
        mRenderDevice.mGraphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
        mRenderDevice.mGraphicsQueueIndex =
            vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
        mRenderDevice.mPresentationQueue = vkbDevice.get_queue(vkb::QueueType::present).value();
        mRenderDevice.mPresentationQueueIndex =
            vkbDevice.get_queue_index(vkb::QueueType::present).value();

        VmaAllocatorCreateInfo allocatorInfo = {
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = *mRenderDevice.mPhysicalDevice,
            .device = *mRenderDevice.mLogicalDevice,
            .pAllocationCallbacks = sj::g_vkAllocationFns,
            .instance = *mVkInstance,
        };
        vmaCreateAllocator(&allocatorInfo, &mRenderDevice.mAllocator);

        // Create the vulkan swap chain connected to the current window and device
        m_swapChain.Init(mRenderDevice, *mSurface, m_display->GetViewportSize());
        CreateRenderImageResources(m_display->GetViewportSize());

        CreateGlobalDescriptorSetlayout();

        m_defaultPipeline =
            sj::vulkan::MakeDefaultMeshPipeline(mRenderDevice.mLogicalDevice,
                                                m_globalUBODescriptorSetLayout,
                                                m_drawImage.GetImageFormat(),
                                                m_depthImage.GetImageFormat(),
                                                "Data/Engine/Shaders/Default.vert.spv",
                                                "Data/Engine/Shaders/Default.frag.spv");

        CreateCommandPools();
        m_immediateCommandContext =
            sj::vulkan::ImmediateCommandContext(mRenderDevice.mLogicalDevice,
                                                mRenderDevice.mGraphicsQueueIndex,
                                                mRenderDevice.mGraphicsQueue);

        m_dummyMeshBuffers.Init("Data/Engine/viking_room.sj_mesh",
                                mRenderDevice.mAllocator,
                                m_immediateCommandContext);

        m_dummyTextureResource = sj::vulkan::TextureResource("Data/Engine/viking_room.sj_tex",
                                                             mRenderDevice,
                                                             m_immediateCommandContext);

        CreateGlobalUniformBuffers();

        CreateGlobalUBODescriptorPool();

#ifndef SJ_GOLD
        CreateImGuiDescriptorPool();
#endif // !SJ_GOLD

        CreateGlobalUBODescriptorSets();

        m_frameData.Init(*mRenderDevice.mLogicalDevice, m_graphicsCommandPool);
    }

    ~Renderer()
    {
        mRenderDevice.mLogicalDevice.waitIdle();

        if(mImGuiInitialized)
            ImGui_ImplVulkan_Shutdown();

        VkDevice logicalDevice = *mRenderDevice.mLogicalDevice;

        m_frameData.DeInit(mRenderDevice);

        vkDestroyCommandPool(logicalDevice, m_graphicsCommandPool, sj::g_vkAllocationFns);

        m_globalDescriptorAllocator.DeInit(logicalDevice);

#ifndef SJ_GOLD
        m_imguiDescriptorAllocator.DeInit(logicalDevice);
#endif

        vkDestroyDescriptorSetLayout(logicalDevice,
                                     m_globalUBODescriptorSetLayout,
                                     sj::g_vkAllocationFns);

        m_dummyMeshBuffers.DeInit(mRenderDevice.mAllocator);
    }

    void NewFrame() {}
    void Process(float _){}
    void EndFrame() {}

    static auto Bootstrap(Window* display) -> std::tuple<vkb::Instance,
                                                         VkSurfaceKHR,
                                                         VkDebugUtilsMessengerEXT,
                                                         vkb::PhysicalDevice,
                                                         vkb::Device>
    {
        vkb::InstanceBuilder builder;
        vkb::Instance vkbInstance =
            builder.set_app_name("SJ Game")
                .request_validation_layers(g_IsDebugBuild) // todo- why cause crash on boot?
#ifndef SJ_GOLD
                .set_debug_callback(VulkanDebugLogCallback)
#endif
                .require_api_version(1, 4, 0)
                .build()
                .value();

        SJ_ENGINE_LOG_INFO("Vulkan Instance Created. Version: {}", vkbInstance.instance_version);

        VkSurfaceKHR surface = display->CreateWindowSurface(vkbInstance);

#ifndef SJ_GOLD
        VkDebugUtilsMessengerEXT debugMessenger = vkbInstance.debug_messenger;
#endif

        // vulkan 1.3 features
        VkPhysicalDeviceVulkan13Features features1_3 {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
        features1_3.dynamicRendering = true;
        features1_3.synchronization2 = true;

        // vulkan 1.2 features
        VkPhysicalDeviceVulkan12Features features1_2 {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
        features1_2.bufferDeviceAddress = true;
        features1_2.descriptorIndexing = true;

        VkPhysicalDeviceFeatures features {.samplerAnisotropy = VK_TRUE};

        // use vkbootstrap to select a gpu.
        vkb::PhysicalDeviceSelector selector {vkbInstance};
        vkb::PhysicalDevice physicalDevice = selector.set_minimum_version(1, 3)
                                                 .set_required_features_13(features1_3)
                                                 .set_required_features_12(features1_2)
                                                 .set_required_features(features)
                                                 .set_surface(surface)
                                                 .select()
                                                 .value();
        SJ_ENGINE_LOG_INFO("Selected GPU {}", physicalDevice.name.c_str());

        vkb::DeviceBuilder deviceBuilder {physicalDevice};
        vkb::Device logicalDevice = deviceBuilder.build().value();

        return {vkbInstance,
                surface,
                debugMessenger,
                std::move(physicalDevice),
                std::move(logicalDevice)};
    }

    void Render(const Mat44& cameraMatrix)
    {
        m_frameCount++;
        const uint32_t frameIdx = m_frameCount % sj::vulkan::kMaxFramesInFlight;
        sj::vulkan::FrameGlobals currFrameData = m_frameData.GetFrameGlobals(frameIdx);

        vkWaitForFences(*mRenderDevice.mLogicalDevice,
                        1,
                        &currFrameData.fence,
                        VK_TRUE,
                        std::numeric_limits<uint64_t>::max());

        vk::SwapchainKHR swapChain = m_swapChain.GetSwapChain();
        uint32_t imageIndex = 0;
        VkResult res = vkAcquireNextImageKHR(*mRenderDevice.mLogicalDevice,
                                             swapChain,
                                             std::numeric_limits<uint64_t>::max(),
                                             currFrameData.presentCompleteSemaphore,
                                             VK_NULL_HANDLE,
                                             &imageIndex);

        vk::Semaphore currRenderFinishedSemaphore =
            m_swapChain.GetImageRenderCompleteSemaphore(imageIndex);

        if(res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
        {
            OnWindowResize();

            return;
        }
        else if(res != VK_SUCCESS)
        {
            SJ_ASSERT(false, "Failed to acquire swap chain image.");
        }

        // Reset fence when we know we're going to be able to draw this frame
        vkResetFences(*mRenderDevice.mLogicalDevice, 1, &currFrameData.fence);

        vkResetCommandBuffer(currFrameData.cmd, 0);

        UpdateUniformBuffer(currFrameData.globalUniformBuffer.GetMappedMemory(), cameraMatrix);

        VkImage swapChainImage = m_swapChain.GetImage(imageIndex);
        VkImageView swapChainImageView = m_swapChain.GetImageView(imageIndex);
        RecordDrawCommands(currFrameData, swapChainImage, swapChainImageView);

        vk::CommandBufferSubmitInfo submitCommandInfo(currFrameData.cmd);

        vk::SemaphoreSubmitInfo waitInfo(currFrameData.presentCompleteSemaphore,
                                         1,
                                         vk::PipelineStageFlagBits2::eColorAttachmentOutput);
        vk::SemaphoreSubmitInfo signalInfo(currRenderFinishedSemaphore,
                                           1,
                                           vk::PipelineStageFlagBits2::eAllGraphics);

        vk::SubmitInfo2 submitInfo({}, waitInfo, submitCommandInfo, signalInfo);

        mRenderDevice.mGraphicsQueue.submit2(submitInfo, currFrameData.fence);

        vk::PresentInfoKHR presentInfo(currRenderFinishedSemaphore, swapChain, imageIndex);

        vk::Result presentRes = mRenderDevice.mPresentationQueue.presentKHR(presentInfo);
        if(presentRes == vk::Result::eErrorOutOfDateKHR || presentRes == vk::Result::eSuboptimalKHR)
        {
            OnWindowResize();
            return;
        }
        else if(res != VK_SUCCESS)
        {
            SJ_ASSERT(false, "Failed to acquire swap chain image.");
        }
    }

    sj::vulkan::RenderDevice* GetRenderDevice()
    {
        return &mRenderDevice;
    }

    void InitImGui()
    {
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = *mVkInstance;
        init_info.PhysicalDevice = *mRenderDevice.mPhysicalDevice;
        init_info.Device = *mRenderDevice.mLogicalDevice;
        init_info.QueueFamily = mRenderDevice.mGraphicsQueueIndex;
        init_info.Queue = mRenderDevice.mGraphicsQueue;
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = m_imguiDescriptorAllocator.GetPool();
        init_info.Allocator = nullptr;
        init_info.MinImageCount = sj::vulkan::kMaxFramesInFlight;
        init_info.ImageCount = sj::vulkan::kMaxFramesInFlight;
        init_info.CheckVkResultFn = CheckImguiVulkanResult;

        VkFormat swapchainImageFormat = static_cast<VkFormat>(m_swapChain.GetImageFormat());
        init_info.UseDynamicRendering = true;
        init_info.PipelineRenderingCreateInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
        init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swapchainImageFormat;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&init_info);
        mImGuiInitialized = true;
    }

    /**
     * Global state shared by all shaders
     */
    struct GlobalUniformBufferObject
    {
        Mat44 model;
        Mat44 view;
        Mat44 projection;
    };

private:
    /**
     * Callback function that allows the Vulkan API to use the engine's logging system
     * @note See Vulkan API for description of arguments
     */
    static VKAPI_ATTR VkBool32 VKAPI_CALL
    VulkanDebugLogCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                           VkDebugUtilsMessageTypeFlagsEXT message_type,
                           const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                           void* user_data)
    {
        if(severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            SJ_ENGINE_LOG_TRACE("Vulkan Validation Layer message: {}", callback_data->pMessage);
        }
        else if(severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            SJ_ENGINE_LOG_INFO("Vulkan Validation Layer message: {}", callback_data->pMessage);
        }
        else if(severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            SJ_ENGINE_LOG_WARN("Vulkan Validation Layer message: {}", callback_data->pMessage);
        }
        else if(severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            SJ_ENGINE_LOG_ERROR("Vulkan Validation Layer message: {}", callback_data->pMessage);
        }

        return VK_FALSE;
    }

    void CreateRenderImageResources(sj::Vec2 windowSize)
    {
        VmaAllocationCreateInfo renderImageAllocInfo = {};
        renderImageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        renderImageAllocInfo.requiredFlags =
            VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkExtent3D drawImageExtent = {static_cast<uint32_t>(windowSize.GetX()),
                                      static_cast<uint32_t>(windowSize.GetY()),
                                      1};

        // Draw Target
        {
            vk::ImageUsageFlags drawImageUsages =
                vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst |
                vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment;

            m_drawImage = sj::vulkan::ImageResource(mRenderDevice.mAllocator,
                                                    renderImageAllocInfo,
                                                    drawImageExtent,
                                                    vk::Format::eR16G16B16A16Sfloat,
                                                    drawImageUsages,
                                                    vk::ImageTiling::eOptimal);

            m_drawImageView = m_drawImage.MakeImageView(mRenderDevice.mLogicalDevice,
                                                        vk::ImageAspectFlagBits::eColor);
        }

        // Depth Target
        {
            m_depthImage =
                sj::vulkan::ImageResource(mRenderDevice.mAllocator,
                                          renderImageAllocInfo,
                                          drawImageExtent,
                                          vk::Format::eD32Sfloat,
                                          vk::ImageUsageFlagBits::eDepthStencilAttachment,
                                          vk::ImageTiling::eOptimal);

            m_depthImageView = m_depthImage.MakeImageView(mRenderDevice.mLogicalDevice,
                                                          vk::ImageAspectFlagBits::eDepth);
        }
    }

    void CreateCommandPools()
    {
        VkCommandPoolCreateInfo poolInfo {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = mRenderDevice.mGraphicsQueueIndex;

        VkResult res = vkCreateCommandPool(*mRenderDevice.mLogicalDevice,
                                           &poolInfo,
                                           sj::g_vkAllocationFns,
                                           &m_graphicsCommandPool);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to create graphics command pool");
    }

    void CreateGlobalDescriptorSetlayout()
    {
        scratchpad_scope scope = ThreadContext::GetScratchpad();

        sj::vulkan::DescriptorLayoutBuilder builder(&scope.get_allocator());
        builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
        builder.AddBinding(1,
                           VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                           VK_SHADER_STAGE_FRAGMENT_BIT);

        m_globalUBODescriptorSetLayout = builder.Build(*mRenderDevice.mLogicalDevice, nullptr);
    }

    void CreateGlobalUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(GlobalUniformBufferObject);

        for(size_t i = 0; i < sj::vulkan::kMaxFramesInFlight; i++)
        {
            // This might not actaully end up being host visible:
            // https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/usage_patterns.html
            m_frameData.globalUniformBuffers[i] = sj::vulkan::BufferResource(
                mRenderDevice.mAllocator,
                bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                    VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
                    VMA_ALLOCATION_CREATE_MAPPED_BIT);
        }
    }

    void CreateGlobalUBODescriptorPool()
    {
        std::array poolSizes {
            VkDescriptorPoolSize {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                  .descriptorCount = sj::vulkan::kMaxFramesInFlight},
            VkDescriptorPoolSize {.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                  .descriptorCount = sj::vulkan::kMaxFramesInFlight}};

        m_globalDescriptorAllocator.InitPool(*mRenderDevice.mLogicalDevice,
                                             sj::vulkan::kMaxFramesInFlight,
                                             poolSizes,
                                             VkDescriptorPoolCreateFlags {});
    }

#ifndef SJ_GOLD
    void CreateImGuiDescriptorPool()
    {
        std::array<VkDescriptorPoolSize, 11> pool_sizes = {
            VkDescriptorPoolSize {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            VkDescriptorPoolSize {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            VkDescriptorPoolSize {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            VkDescriptorPoolSize {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
            VkDescriptorPoolSize {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
            VkDescriptorPoolSize {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
            VkDescriptorPoolSize {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            VkDescriptorPoolSize {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            VkDescriptorPoolSize {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            VkDescriptorPoolSize {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            VkDescriptorPoolSize {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

        m_imguiDescriptorAllocator.InitPool(*mRenderDevice.mLogicalDevice,
                                            1,
                                            pool_sizes,
                                            VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
    }
#endif // !SJ_GOLD

    void CreateGlobalUBODescriptorSets()
    {
        std::array<VkDescriptorSetLayout, sj::vulkan::kMaxFramesInFlight> layouts {};
        std::ranges::fill(layouts, m_globalUBODescriptorSetLayout);

        m_globalDescriptorAllocator.Allocate(*mRenderDevice.mLogicalDevice,
                                             layouts,
                                             m_frameData.globalUBODescriptorSets);

        for(size_t i = 0; i < sj::vulkan::kMaxFramesInFlight; i++)
        {
            VkDescriptorBufferInfo bufferInfo {};
            bufferInfo.buffer = m_frameData.globalUniformBuffers[i].GetBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(GlobalUniformBufferObject);

            VkDescriptorImageInfo imageInfo {};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = m_dummyTextureResource.GetImageView();
            imageInfo.sampler = m_dummyTextureResource.GetSampler();

            std::array descriptorWrites {
                VkWriteDescriptorSet {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                      .dstSet = m_frameData.globalUBODescriptorSets[i],
                                      .dstBinding = 0,
                                      .dstArrayElement = 0,
                                      .descriptorCount = 1,
                                      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                      .pBufferInfo = &bufferInfo},
                VkWriteDescriptorSet {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                      .dstSet = m_frameData.globalUBODescriptorSets[i],
                                      .dstBinding = 1,
                                      .dstArrayElement = 0,
                                      .descriptorCount = 1,
                                      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                      .pImageInfo = &imageInfo}

            };

            vkUpdateDescriptorSets(*mRenderDevice.mLogicalDevice,
                                   static_cast<uint32_t>(descriptorWrites.size()),
                                   descriptorWrites.data(),
                                   0,
                                   nullptr);
        }
    }

    void DrawImGui(VkCommandBuffer cmd, VkImageView targetImageView, VkExtent2D targetImageExtent)
    {
        VkRenderingAttachmentInfo colorAttachment =
            sj::vulkan::MakeAttachmentInfo(targetImageView,
                                           std::nullopt,
                                           vk::ImageLayout::eColorAttachmentOptimal);

        VkRenderingInfo renderInfo =
            sj::vulkan::MakeRenderingInfo(targetImageExtent, &colorAttachment, nullptr);

        vkCmdBeginRendering(cmd, &renderInfo);
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
        vkCmdEndRendering(cmd);
    }

    void DrawBackground(VkCommandBuffer cmd)
    {
        // Blue flashy clear color
        VkClearColorValue clearValue;
        float flash = std::abs(std::sin((float)m_frameCount / 240.f));
        clearValue = {{0.0f, 0.0f, flash, 1.0f}};

        VkImageSubresourceRange clearRange =
            sj::vulkan::MakeImageSubResourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

        vkCmdClearColorImage(cmd,
                             m_drawImage.GetImage(),
                             VK_IMAGE_LAYOUT_GENERAL,
                             &clearValue,
                             1,
                             &clearRange);
    }

    void DrawGeometry(VkCommandBuffer cmd, VkDescriptorSet globalDescriptorSet)
    {
        VkRenderingAttachmentInfo colorAttachment =
            sj::vulkan::MakeAttachmentInfo(m_drawImageView,
                                           std::nullopt,
                                           vk::ImageLayout::eColorAttachmentOptimal);

        VkRenderingAttachmentInfo depthAttachment =
            sj::vulkan::MakeDepthAttachmentInfo(m_depthImageView,
                                                vk::ImageLayout::eDepthAttachmentOptimal);

        VkRenderingInfo renderInfo = sj::vulkan::MakeRenderingInfo(
            {m_drawImage.GetExtent().width, m_drawImage.GetExtent().height},
            &colorAttachment,
            &depthAttachment);

        vkCmdBeginRendering(cmd, &renderInfo);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_defaultPipeline.pipeline);

        std::array<VkDescriptorSet, 1> descriptorSets = {globalDescriptorSet};
        vkCmdBindDescriptorSets(cmd,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                *m_defaultPipeline.layout,
                                0,
                                descriptorSets.size(),
                                descriptorSets.data(),
                                0,
                                nullptr);

        VkViewport viewport {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapChain.GetExtent().width);
        viewport.height = static_cast<float>(m_swapChain.GetExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor {};
        scissor.offset = {.x = 0, .y = 0};
        scissor.extent = m_swapChain.GetExtent();
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        std::array vertexBuffers = {m_dummyMeshBuffers.GetVertexBuffer()};
        std::array<VkDeviceSize, 1> offsets = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers.data(), offsets.data());
        vkCmdBindIndexBuffer(cmd,
                             m_dummyMeshBuffers.GetIndexBuffer(),
                             0,
                             m_dummyMeshBuffers.GetIndexType());
        vkCmdDrawIndexed(cmd, m_dummyMeshBuffers.GetIndexCount(), 1, 0, 0, 0);
        vkCmdEndRendering(cmd);
    }

    void RecordDrawCommands(sj::vulkan::FrameGlobals& frameData,
                            VkImage swapchainImage,
                            VkImageView swapchainImageView)
    {
        VkCommandBufferBeginInfo beginInfo =
            sj::vulkan::MakeCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        VkResult res = vkBeginCommandBuffer(frameData.cmd, &beginInfo);
        SJ_ASSERT(res == VK_SUCCESS, "Failed to start command buffer!");

        // Make render target images
        sj::vulkan::TransitionImage(frameData.cmd,
                                    m_drawImage.GetImage(),
                                    VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_GENERAL);

        DrawBackground(frameData.cmd);

        sj::vulkan::TransitionImage(frameData.cmd,
                                    m_drawImage.GetImage(),
                                    VK_IMAGE_LAYOUT_GENERAL,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        sj::vulkan::TransitionImage(frameData.cmd,
                                    m_depthImage.GetImage(),
                                    VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

        DrawGeometry(frameData.cmd, frameData.globalDescriptorSet);

        // transition the draw image and the swapchain image into their correct transfer layouts
        sj::vulkan::TransitionImage(frameData.cmd,
                                    m_drawImage.GetImage(),
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        sj::vulkan::TransitionImage(frameData.cmd,
                                    swapchainImage,
                                    VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // Copy working image into swapchain
        sj::vulkan::CopyImageToImage(frameData.cmd,
                                     m_drawImage.GetImage(),
                                     swapchainImage,
                                     m_drawImage.GetExtent2D(),
                                     m_swapChain.GetExtent());

        // Put swapchain image back into color attachment mode
        sj::vulkan::TransitionImage(frameData.cmd,
                                    swapchainImage,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

#ifndef SJ_GOLD
        DrawImGui(frameData.cmd, swapchainImageView, m_swapChain.GetExtent());
#endif

        // Make swapchain image ready for presentation
        sj::vulkan::TransitionImage(frameData.cmd,
                                    swapchainImage,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        res = vkEndCommandBuffer(frameData.cmd);
        SJ_ASSERT(res == VK_SUCCESS, "Failed to record command buffer!");
    }

    void UpdateUniformBuffer(void* bufferMem, const Mat44& cameraMatrix)
    {
        GlobalUniformBufferObject ubo {};
        ubo.model = Mat44(kIdentityTag);
        ubo.view = cameraMatrix.AffineInverse();

        VkExtent2D extent = m_swapChain.GetExtent();
        const float aspectRatio =
            static_cast<float>(extent.width) / static_cast<float>(extent.height);
        ubo.projection = PerspectiveProjection(ToRadians(45.0f), aspectRatio, 10000.0f, 0.1f);
        memcpy(bufferMem, &ubo, sizeof(ubo));
    }

    void OnWindowResize()
    {
        mRenderDevice.mLogicalDevice.waitIdle();
        VkDevice device = *mRenderDevice.mLogicalDevice;

        m_swapChain.Recreate(mRenderDevice, *mSurface, m_display->GetViewportSize());

        CreateRenderImageResources(m_display->GetViewportSize());

        m_frameData.DestroySyncPrimitives(device);
        m_frameData.CreateSyncPrimitives(device);
    }

    Window* m_display = nullptr;

    bool mImGuiInitialized = false;

    vk::raii::Context mVkContext;
    vk::raii::Instance mVkInstance {nullptr};
    vk::raii::SurfaceKHR mSurface {nullptr};

    sj::vulkan::RenderDevice mRenderDevice;

    sj::vulkan::ImageResource m_drawImage;
    vk::raii::ImageView m_drawImageView {nullptr};

    sj::vulkan::ImageResource m_depthImage;
    vk::raii::ImageView m_depthImageView {nullptr};

    /** Used for image presentation */
    sj::vulkan::SwapChain m_swapChain;

    /** Pipeline used to describe rendering process */
    sj::vulkan::PipelineResource m_defaultPipeline {};

    VkCommandPool m_graphicsCommandPool {};

    sj::vulkan::MeshBuffers m_dummyMeshBuffers;

    sj::vulkan::TextureResource m_dummyTextureResource;

    sj::vulkan::DescriptorAllocator m_globalDescriptorAllocator;
    VkDescriptorSetLayout m_globalUBODescriptorSetLayout {};

    sj::vulkan::ImmediateCommandContext m_immediateCommandContext;

    sj::vulkan::FrameResources m_frameData {};

    uint32_t m_frameCount = 0;

#ifndef SJ_GOLD
    sj::vulkan::DescriptorAllocator m_imguiDescriptorAllocator;

    /** Handle to manage Vulkan's debug callbacks */
    vk::raii::DebugUtilsMessengerEXT mVkDebugMessenger {nullptr};
#endif
};
} // namespace sj