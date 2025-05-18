// Parent Include
#include <ScrewjankEngine/rendering/Renderer.hpp>

// Screwjank Headers
#include <ScrewjankEngine/framework/Window.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanHelpers.hpp>
#include <ScrewjankShared/utils/Assert.hpp>
#include <ScrewjankShared/utils/Log.hpp>

// Shared Headers
#include <ScrewjankShared/Math/Helpers.hpp>
#include <ScrewjankShared/DataDefinitions/Assets/Texture.hpp>
#include <ScrewjankShared/DataDefinitions/Assets/Model.hpp>
#include <ScrewjankShared/io/File.hpp>
#include <cstddef>
#include <cstring>
#include <vulkan/vulkan_core.h>

// Library Headers
#ifndef SJ_GOLD
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#endif

// STD Headers
#include <array>

import sj.engine.system.memory;
import sj.shared.containers;

namespace sj
{
    static void CheckImguiVulkanResult(VkResult res)
    {
        SJ_ASSERT(res == VK_SUCCESS, "ImGui Vulkan operation failed!");
    }

    MemSpace<FreeListAllocator>* Renderer::WorkBuffer()
    {
        static MemSpace zone(MemorySystem::GetRootMemSpace(), 4_MiB, "Renderer Work Buffer");
        return &zone;
    }

    Renderer* Renderer::GetInstance()
    {
        static Renderer s_renderer;
        return &s_renderer;
    }

    void Renderer::Render(const Mat44& cameraMatrix)
    {
        #ifndef SJ_GOLD
        ImGui::Render();
        #endif // !SJ_GOLD


        const uint32_t frameIdx = m_frameCount % kMaxFramesInFlight;

        VkCommandBuffer currCommandBuffer = m_frameData.commandBuffers[frameIdx];
        VkFence currFence = m_frameData.inFlightFences[frameIdx];
        VkSemaphore currImageAvailableSemaphore = m_frameData.imageAvailableSemaphores[frameIdx];
        VkSemaphore currRenderFinishedSemaphore = m_frameData.renderFinishedSemaphores[frameIdx];

        vkWaitForFences(m_renderDevice.GetLogicalDevice(),
                        1,
                        &currFence,
                        VK_TRUE,
                        std::numeric_limits<uint64_t>::max());

        uint32_t imageIndex;
        VkResult res = vkAcquireNextImageKHR(m_renderDevice.GetLogicalDevice(),
                                             m_swapChain.GetSwapChain(),
                                             std::numeric_limits<uint64_t>::max(),
                                             currImageAvailableSemaphore,
                                             VK_NULL_HANDLE,
                                             &imageIndex);

        if(res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
        {
            m_swapChain.Recreate(m_renderDevice.GetPhysicalDevice(),
                                 m_renderDevice.GetLogicalDevice(),
                                 m_renderingSurface,
                                 m_defaultRenderPass);

            return;
        }
        else if(res != VK_SUCCESS)
        {
            SJ_ASSERT(false, "Failed to acquire swap chain image.");
        }

        UpdateUniformBuffer(m_frameData.globalUniformBuffersMapped[frameIdx], cameraMatrix);

        // Reset fence when we know we're going to be able to draw this frame
        vkResetFences(m_renderDevice.GetLogicalDevice(), 1, &currFence);

        vkResetCommandBuffer(currCommandBuffer, 0);

        RecordCommandBuffer(currCommandBuffer, frameIdx, imageIndex);

        VkSubmitInfo submitInfo {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {currImageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &currCommandBuffer;

        VkSemaphore signalSemaphores[] = {currRenderFinishedSemaphore};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        res = vkQueueSubmit(m_renderDevice.GetGraphicsQueue(), 1, &submitInfo, currFence);
        SJ_ASSERT(res == VK_SUCCESS, "Failed to submit draw command buffer!");

        VkPresentInfoKHR presentInfo {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {m_swapChain.GetSwapChain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        res = vkQueuePresentKHR(m_renderDevice.GetPresentationQueue(), &presentInfo);
        if(res == VK_ERROR_OUT_OF_DATE_KHR)
        {
            m_swapChain.Recreate(m_renderDevice.GetPhysicalDevice(),
                                 m_renderDevice.GetLogicalDevice(),
                                 m_renderingSurface,
                                 m_defaultRenderPass);

            return;
        }
        else if(res != VK_SUCCESS)
        {
            SJ_ASSERT(false, "Failed to acquire swap chain image.");
        }

        m_frameCount++;
    }

    void Renderer::Init() 
    {
        InitializeVulkan();

        // Create rendering surface
        CreateRenderSurface();

        // Select physical device and create and logical render device
        m_renderDevice.Init(m_renderingSurface);

        // Create the vulkan swap chain connected to the current window and device
        m_swapChain.Init(m_renderDevice.GetPhysicalDevice(),
                         m_renderDevice.GetLogicalDevice(),
                         m_renderingSurface);

        CreateRenderPass();

        CreateGlobalDescriptorSetlayout();

        m_defaultPipeline.Init(m_renderDevice.GetLogicalDevice(),
                               m_swapChain.GetExtent(),
                               m_defaultRenderPass,
                               m_globalUBODescriptorSetLayout,
                               "Data/Engine/Shaders/Default.vert.spv",
                               "Data/Engine/Shaders/Default.frag.spv");

        m_swapChain.InitFrameBuffers(m_renderDevice.GetLogicalDevice(), m_defaultRenderPass);

        CreateCommandPools();
        
        LoadDummyModel();
        CreateDummyTextureImage();

        m_dummyTextureImageView = CreateImageView(m_renderDevice.GetLogicalDevice(),
                                                  m_dummyTextureImage,
                                                  VK_FORMAT_R8G8B8A8_SRGB,
                                                  VK_IMAGE_ASPECT_COLOR_BIT);

        CreateDummyTextureSampler();

        CreateGlobalUniformBuffers();

        CreateGlobalUBODescriptorPool();
        
        #ifndef SJ_GOLD
        CreateImGuiDescriptorPool();
        #endif // !SJ_GOLD

        CreateGlobalUBODescriptorSets();

        m_frameData.Init(m_renderDevice.GetLogicalDevice(), m_graphicsCommandPool);

        DeviceQueueFamilyIndices indices =
            VulkanRenderDevice::GetDeviceQueueFamilyIndices(m_renderDevice.GetPhysicalDevice());

        #ifndef SJ_GOLD
        // Init ImGui
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = m_vkInstance;
        init_info.PhysicalDevice = m_renderDevice.GetPhysicalDevice();
        init_info.Device = m_renderDevice.GetLogicalDevice();
        init_info.QueueFamily = *indices.graphicsFamilyIndex;
        init_info.Queue = m_renderDevice.GetGraphicsQueue();
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = m_imguiDescriptorPool;
        init_info.Allocator = nullptr;
        init_info.MinImageCount = kMaxFramesInFlight;
        init_info.ImageCount = kMaxFramesInFlight;
        init_info.CheckVkResultFn = CheckImguiVulkanResult;

        init_info.RenderPass = m_defaultRenderPass;

        ImGui_ImplVulkan_Init(&init_info);
        #endif
    }

    void Renderer::CreateGlobalDescriptorSetlayout()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding {};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

        VkDescriptorSetLayoutBinding samplerLayoutBinding {};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array bindings = {uboLayoutBinding, samplerLayoutBinding};

        VkDescriptorSetLayoutCreateInfo layoutInfo {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        VkResult res = vkCreateDescriptorSetLayout(m_renderDevice.GetLogicalDevice(),
                                                   &layoutInfo,
                                                   sj::g_vkAllocationFns,
                                                   &m_globalUBODescriptorSetLayout);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to create descriptor set layout");
    }


    void Renderer::DeInit()
    {
        VkDevice logicalDevice = m_renderDevice.GetLogicalDevice();
        vkDeviceWaitIdle(logicalDevice);

        #ifndef SJ_GOLD
        ImGui_ImplVulkan_Shutdown();
        #endif // !SJ_GOLD


        m_frameData.DeInit(logicalDevice);

        vkDestroyCommandPool(logicalDevice, m_graphicsCommandPool, sj::g_vkAllocationFns);

        m_swapChain.DeInit(logicalDevice);
        
        vkDestroySampler(logicalDevice, m_dummyTextureSampler, sj::g_vkAllocationFns);
        vkDestroyImageView(logicalDevice, m_dummyTextureImageView, sj::g_vkAllocationFns);
        vkDestroyImage(logicalDevice, m_dummyTextureImage, sj::g_vkAllocationFns);
        vkFreeMemory(logicalDevice, m_dummyTextureImageMemory, sj::g_vkAllocationFns);

        vkDestroyDescriptorPool(logicalDevice, m_globalUBODescriptorPool, sj::g_vkAllocationFns);

        if constexpr(g_IsDebugBuild)
        {
            vkDestroyDescriptorPool(logicalDevice, m_imguiDescriptorPool, sj::g_vkAllocationFns);
        }

        vkDestroyDescriptorSetLayout(logicalDevice, m_globalUBODescriptorSetLayout, sj::g_vkAllocationFns);

        m_defaultPipeline.DeInit();

        vkDestroyRenderPass(logicalDevice, m_defaultRenderPass, sj::g_vkAllocationFns);

        vkDestroyBuffer(logicalDevice, m_dummyVertexBuffer, sj::g_vkAllocationFns);
        vkFreeMemory(logicalDevice, m_dummyVertexBufferMem, sj::g_vkAllocationFns);

        vkDestroyBuffer(logicalDevice, m_dummyIndexBuffer, sj::g_vkAllocationFns);
        vkFreeMemory(logicalDevice, m_dummyIndexBufferMem, sj::g_vkAllocationFns);

        // Important: All things attached to the device need to be torn down first
        m_renderDevice.DeInit();

        vkDestroySurfaceKHR(m_vkInstance, m_renderingSurface, sj::g_vkAllocationFns);

        if constexpr(g_IsDebugBuild)
        {
            auto messenger_destroy_func = (PFN_vkDestroyDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr(m_vkInstance, "vkDestroyDebugUtilsMessengerEXT");

            SJ_ASSERT(messenger_destroy_func != nullptr,
                      "Failed to load Vulkan Debug messenger destroy function");

            messenger_destroy_func(m_vkInstance, m_vkDebugMessenger, sj::g_vkAllocationFns);
        }

        vkDestroyInstance(m_vkInstance, sj::g_vkAllocationFns);
    }

    VkInstance Renderer::GetVkInstanceHandle() const
    {
        return m_vkInstance;
    }

    void Renderer::StartRenderFrame()
    {
        Viewport viewport = Window::GetInstance()->GetViewportSize();

        #ifndef SJ_GOLD
        {
            MemSpaceScope _(MemorySystem::GetDebugMemSpace());

            ImGui::GetIO().DisplaySize = {(float)viewport.Width, (float)viewport.Height};
            
            // Start the Dear ImGui frame
            ImGui_ImplVulkan_NewFrame();
            ImGui::NewFrame();
        }
        #endif // !SJ_GOLD
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::VulkanDebugLogCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
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

    bool Renderer::HasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void Renderer::InitializeVulkan()
    {
        VkApplicationInfo app_info;
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pNext = nullptr;
        app_info.pApplicationName = "Screwjank Engine Game";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "Screwjank Engine";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_2;

        VkInstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = {};
        create_info.pApplicationInfo = &app_info;
        create_info.enabledLayerCount = {};
        create_info.ppEnabledLayerNames = nullptr;

        // Get extension count and names
        std::span<const char*> required_extensions = Window::GetInstance()->GetRequiredVulkanExtenstions();
        create_info.ppEnabledExtensionNames = required_extensions.data();
        create_info.enabledExtensionCount = required_extensions.size();

#ifndef SJ_GOLD
        create_info.enabledExtensionCount++;
        dynamic_vector<const char*> debug_required_extensions(
            std::from_range_t{},
            required_extensions,
            MemorySystem::GetDebugMemSpace()
        );

        debug_required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        create_info.ppEnabledExtensionNames = debug_required_extensions.data();

        static std::array layers 
        {
            "VK_LAYER_KHRONOS_validation"
        };

        EnableValidationLayers(layers);

        create_info.enabledLayerCount = (uint32_t)layers.size();
        create_info.ppEnabledLayerNames = layers.data();
#endif

        // Create the vulkan instance
        {
            VkResult result = vkCreateInstance(
                &create_info, 
                sj::g_vkAllocationFns,
                &m_vkInstance
            );

            SJ_ASSERT(result == VK_SUCCESS,
                    "Vulkan instance creation failed with error code {}",
                    (int)result);
        }

        // Compile-time check to enable debug messaging
        if constexpr(g_IsDebugBuild)
        {
            EnableDebugMessaging();
        }

        // Log success
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        SJ_ENGINE_LOG_INFO("Vulkan loaded with {} extensions supported", extensionCount);
    }

    void Renderer::CreateRenderSurface()
    {
        m_renderingSurface = Window::GetInstance()->CreateWindowSurface(m_vkInstance);
    }

    void Renderer::CreateRenderPass()
    {
        VkAttachmentDescription depthAttachment {};
        depthAttachment.format = FindDepthFormat(m_renderDevice.GetPhysicalDevice());
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachment {};
        colorAttachment.format = m_swapChain.GetImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference depthAttachmentRef {};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | 
                                   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array attachments = {colorAttachment, depthAttachment};
        VkRenderPassCreateInfo renderPassInfo {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VkResult res = vkCreateRenderPass(m_renderDevice.GetLogicalDevice(),
                                          &renderPassInfo,
                                          sj::g_vkAllocationFns,
                                          &m_defaultRenderPass);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to create render pass.");
    }

    VkCommandBuffer Renderer::BeginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_graphicsCommandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_renderDevice.GetLogicalDevice(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void
    Renderer::EnableValidationLayers(std::span<const char*> required_validation_layers)
    {
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        SJ_ASSERT(layer_count <= 64, "Overflow");
        std::array<VkLayerProperties, 64> available_layers;
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

        // Verify required validation layers are supported
        for(auto layer_name : required_validation_layers)
        {
            bool layer_found = false;

            for(const auto& layer_properties : available_layers)
            {
                if(strcmp(layer_name, layer_properties.layerName) == 0)
                {
                    layer_found = true;
                    break;
                }
            }

            if(!layer_found)
            {
                SJ_ASSERT(false, "Failed to enable vulkan validation layer {}", layer_name);
            }

            SJ_ENGINE_LOG_DEBUG("Enabled Vulkan validation layer: {}", layer_name);
        }
    }

    void Renderer::EnableDebugMessaging()
    {
        // Hook up the debug messenger to instance
        VkDebugUtilsMessengerCreateInfoEXT messenger_create_info = {};
        messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        messenger_create_info.pfnUserCallback = VulkanDebugLogCallback;
        messenger_create_info.pUserData = nullptr;

        // Get extension function pointer
        auto create_function = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            m_vkInstance,
            "vkCreateDebugUtilsMessengerEXT");

        SJ_ASSERT(create_function != nullptr,
                  "Failed to load vulkan extension function vkCreateDebugUtilsMessengerEXT");

        create_function(
            m_vkInstance, 
            &messenger_create_info, 
            sj::g_vkAllocationFns, 
            &m_vkDebugMessenger );
    }

    void Renderer::TransitionImageLayout(VkImage image,
                                         VkImageLayout oldLayout,
                                         VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

        VkImageMemoryBarrier barrier {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;

        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;
        if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
           newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else 
        {
            SJ_ASSERT(false, "unsupported layout transition!");
        }

        vkCmdPipelineBarrier(commandBuffer,
                             sourceStage,
                             destinationStage,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &barrier);

        EndSingleTimeCommands(commandBuffer);
    }

    void
    Renderer::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

        VkBufferImageCopy region {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, 1};

        // "Assuming here that the image has already been transitioned to the layout that is optimal for copying pixels to"
        vkCmdCopyBufferToImage(commandBuffer,
                               buffer,
                               image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1,
                               &region);

        EndSingleTimeCommands(commandBuffer);
    }

    void Renderer::CreateCommandPools()
    {
        DeviceQueueFamilyIndices indices =
            VulkanRenderDevice::GetDeviceQueueFamilyIndices(m_renderDevice.GetPhysicalDevice());

        // Create Graphics Command Pool
        {
            VkCommandPoolCreateInfo poolInfo {};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolInfo.queueFamilyIndex = *(indices.graphicsFamilyIndex);

            VkResult res = vkCreateCommandPool(m_renderDevice.GetLogicalDevice(),
                                               &poolInfo,
                                               sj::g_vkAllocationFns,
                                               &m_graphicsCommandPool);

            SJ_ASSERT(res == VK_SUCCESS, "Failed to create graphics command pool");
        }
    }

    void Renderer::CreateBuffer(VkDeviceSize size,
                                VkBufferUsageFlags usage,
                                VkMemoryPropertyFlags properties,
                                VkBuffer& out_buffer,
                                VkDeviceMemory& out_bufferMemory)
    {
        VkDevice logicalDevice = m_renderDevice.GetLogicalDevice();

        DeviceQueueFamilyIndices indices =
            VulkanRenderDevice::GetDeviceQueueFamilyIndices(m_renderDevice.GetPhysicalDevice());

        std::array<uint32_t, 1> queueFamilyIndices {*indices.graphicsFamilyIndex};
        

        VkBufferCreateInfo bufferInfo {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.queueFamilyIndexCount = 1;
        bufferInfo.pQueueFamilyIndices = queueFamilyIndices.data();

        VkResult res = vkCreateBuffer(logicalDevice, &bufferInfo, sj::g_vkAllocationFns, &out_buffer);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to create vulkan buffer");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(logicalDevice, out_buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(m_renderDevice.GetPhysicalDevice(),
                                                   memRequirements.memoryTypeBits,
                                                   properties);

        res = vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &out_bufferMemory);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to allocate dummy vertex buffer memory");

        vkBindBufferMemory(logicalDevice, out_buffer, out_bufferMemory, 0);
    }

    void Renderer::EndSingleTimeCommands(VkCommandBuffer& commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        VkQueue targetQueue = m_renderDevice.GetGraphicsQueue();
        vkQueueSubmit(targetQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(targetQueue);

        vkFreeCommandBuffers(m_renderDevice.GetLogicalDevice(),
                             m_graphicsCommandPool,
                             1,
                             &commandBuffer);
    }

    void Renderer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

        VkBufferCopy copyRegion {};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        EndSingleTimeCommands(commandBuffer);
    }

    void Renderer::LoadDummyModel()
    {
        File modelFile;
        modelFile.Open("Data/Engine/viking_room.sj_mesh", sj::File::OpenMode::kReadBinary);
        Model header;
        modelFile.Read(&header, sizeof(header));

        // Read verts into GPU memory
        {
            VkDeviceSize bufferSize = sizeof(Vertex) * header.numVerts;
            // Stage vertex data in host visible buffer
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;

            CreateBuffer(bufferSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    stagingBuffer,
                    stagingBufferMemory);

            void* data;
            vkMapMemory(m_renderDevice.GetLogicalDevice(),
                        stagingBufferMemory,
                        0,
                        bufferSize,
                        0,
                        &data);

            // Copy data from file to GPU
            modelFile.Read(data, bufferSize);

            vkUnmapMemory(m_renderDevice.GetLogicalDevice(), stagingBufferMemory);

            CreateBuffer(bufferSize,
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         m_dummyVertexBuffer,
                         m_dummyVertexBufferMem);

            CopyBuffer(stagingBuffer, m_dummyVertexBuffer, bufferSize);

            vkDestroyBuffer(m_renderDevice.GetLogicalDevice(), stagingBuffer, sj::g_vkAllocationFns);
            vkFreeMemory(m_renderDevice.GetLogicalDevice(), stagingBufferMemory, sj::g_vkAllocationFns);
        }

        // Read Indices into GPU memory
        {
            m_dummyIndexBufferIndexCount = header.numIndices;

            VkDeviceSize bufferSize = sizeof(uint16_t) * header.numIndices;

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            CreateBuffer(bufferSize,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer,
                         stagingBufferMemory);

            void* data;
            vkMapMemory(m_renderDevice.GetLogicalDevice(),
                        stagingBufferMemory,
                        0,
                        bufferSize,
                        0,
                        &data);
            // Copy data from file to GPU
            modelFile.Read(data, bufferSize);

            vkUnmapMemory(m_renderDevice.GetLogicalDevice(), stagingBufferMemory);

            CreateBuffer(bufferSize,
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         m_dummyIndexBuffer,
                         m_dummyIndexBufferMem);

            CopyBuffer(stagingBuffer, m_dummyIndexBuffer, bufferSize);

            vkDestroyBuffer(m_renderDevice.GetLogicalDevice(), stagingBuffer, sj::g_vkAllocationFns);
            vkFreeMemory(m_renderDevice.GetLogicalDevice(), stagingBufferMemory, sj::g_vkAllocationFns);
        }

        modelFile.Close();
    }

    void Renderer::CreateDummyTextureImage()
    {
        File textureFile;
        textureFile.Open("Data/Engine/viking_room.sj_tex", sj::File::OpenMode::kReadBinary);
        TextureHeader header;
        textureFile.Read(&header, sizeof(header));

        VkDeviceSize imageSize = header.width * header.height * 4;
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        CreateBuffer(imageSize,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     stagingBuffer,
                     stagingBufferMemory);

        void* data;
        vkMapMemory(m_renderDevice.GetLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
        
        // Read texture data straight into GPU memory
        textureFile.Read(data, imageSize);
        vkUnmapMemory(m_renderDevice.GetLogicalDevice(), stagingBufferMemory);
      
        textureFile.Close();
        CreateImage(m_renderDevice.GetLogicalDevice(), 
                    m_renderDevice.GetPhysicalDevice(), 
                    header.width,
                    header.height,
                    VK_FORMAT_R8G8B8A8_SRGB,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    m_dummyTextureImage,
                    m_dummyTextureImageMemory);
        
        TransitionImageLayout(m_dummyTextureImage,
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        CopyBufferToImage(stagingBuffer, m_dummyTextureImage, header.width, header.height);
        
        TransitionImageLayout(m_dummyTextureImage,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(m_renderDevice.GetLogicalDevice(), stagingBuffer, sj::g_vkAllocationFns);
        vkFreeMemory(m_renderDevice.GetLogicalDevice(), stagingBufferMemory, sj::g_vkAllocationFns);

        return;
    }

    void Renderer::CreateDummyTextureSampler()
    {
        VkPhysicalDeviceProperties properties {};
        vkGetPhysicalDeviceProperties(m_renderDevice.GetPhysicalDevice(), &properties);

        VkSamplerCreateInfo samplerInfo {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        VkResult res = vkCreateSampler(m_renderDevice.GetLogicalDevice(),
                                       &samplerInfo,
                                       sj::g_vkAllocationFns,
                                       &m_dummyTextureSampler);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to create image sampler");
    }

    void Renderer::CreateGlobalUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(GlobalUniformBufferObject);

        for(size_t i = 0; i < kMaxFramesInFlight; i++)
        {
            CreateBuffer(bufferSize,
                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         m_frameData.globalUniformBuffers[i],
                         m_frameData.globalUniformBuffersMemory[i]);

            vkMapMemory(m_renderDevice.GetLogicalDevice(),
                        m_frameData.globalUniformBuffersMemory[i],
                        0,
                        bufferSize,
                        0,
                        &(m_frameData.globalUniformBuffersMapped[i]));
        }
    }

    void Renderer::CreateGlobalUBODescriptorPool()
    {
        std::array<VkDescriptorPoolSize, 2> poolSizes {};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = kMaxFramesInFlight;
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = kMaxFramesInFlight;

        VkDescriptorPoolCreateInfo poolInfo {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = poolSizes.size();
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = kMaxFramesInFlight;

        VkResult res = vkCreateDescriptorPool(m_renderDevice.GetLogicalDevice(),
                                              &poolInfo,
                                              sj::g_vkAllocationFns,
                                              &m_globalUBODescriptorPool);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to create descriptor pool for global UBO");
    }
    
    #ifndef SJ_GOLD
    void Renderer::CreateImGuiDescriptorPool()
    {
        VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                             {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                             {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                             {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                             {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 1;
        poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        poolInfo.pPoolSizes = pool_sizes;

        VkResult res = vkCreateDescriptorPool(m_renderDevice.GetLogicalDevice(),
                                              &poolInfo,
                                              sj::g_vkAllocationFns,
                                              &m_imguiDescriptorPool);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to create descriptor pool for global UBO");
    }
    #endif

    void Renderer::CreateGlobalUBODescriptorSets()
    {
        std::array<VkDescriptorSetLayout, kMaxFramesInFlight> layouts;
        for(VkDescriptorSetLayout& layout : layouts)
        {
            layout = m_globalUBODescriptorSetLayout;
        }

        VkDescriptorSetAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_globalUBODescriptorPool;
        allocInfo.descriptorSetCount = kMaxFramesInFlight;
        allocInfo.pSetLayouts = layouts.data();

        VkDevice logicalDevice = m_renderDevice.GetLogicalDevice();
        VkResult res = vkAllocateDescriptorSets(logicalDevice,
                                                &allocInfo,
                                                m_frameData.globalUBODescriptorSets.data());

        SJ_ASSERT(res == VK_SUCCESS, "Failed to allocate descriptor sets for global UBOs");

        for(size_t i = 0; i < kMaxFramesInFlight; i++)
        {
            VkDescriptorBufferInfo bufferInfo {};
            bufferInfo.buffer = m_frameData.globalUniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(GlobalUniformBufferObject);

            VkDescriptorImageInfo imageInfo {};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = m_dummyTextureImageView;
            imageInfo.sampler = m_dummyTextureSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites {};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = m_frameData.globalUBODescriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = m_frameData.globalUBODescriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(logicalDevice,
                                   static_cast<uint32_t>(descriptorWrites.size()),
                                   descriptorWrites.data(),
                                   0,
                                   nullptr);
        }
    }

    void Renderer::RecordCommandBuffer(VkCommandBuffer buffer,
                                                uint32_t frameIdx,
                                                uint32_t imageIdx)
    {
        VkCommandBufferBeginInfo beginInfo {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        {
            VkResult res = vkBeginCommandBuffer(buffer, &beginInfo);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to create command buffer");
        }

        VkRenderPassBeginInfo renderPassInfo {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_defaultRenderPass;
        renderPassInfo.framebuffer = m_swapChain.GetFrameBuffers()[imageIdx];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapChain.GetExtent();

        std::array<VkClearValue, 2> clearValues {};
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            vkCmdBindPipeline(buffer,
                              VK_PIPELINE_BIND_POINT_GRAPHICS,
                              m_defaultPipeline.GetPipeline());

            VkBuffer vertexBuffers[] = {m_dummyVertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(buffer, m_dummyIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

            VkViewport viewport {};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(m_swapChain.GetExtent().width);
            viewport.height = static_cast<float>(m_swapChain.GetExtent().height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(buffer, 0, 1, &viewport);

            VkRect2D scissor {};
            scissor.offset = {0, 0};
            scissor.extent = m_swapChain.GetExtent();
            vkCmdSetScissor(buffer, 0, 1, &scissor);

            vkCmdBindDescriptorSets(buffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    m_defaultPipeline.GetLayout(),
                                    0,
                                    1,
                                    &m_frameData.globalUBODescriptorSets[frameIdx],
                                    0,
                                    nullptr);

            vkCmdDrawIndexed(buffer, static_cast<uint32_t>(m_dummyIndexBufferIndexCount), 1, 0, 0, 0);

            #ifndef SJ_GOLD
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), buffer);
            #endif
        }
        vkCmdEndRenderPass(buffer);

        {
            VkResult res = vkEndCommandBuffer(buffer);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to record command buffer!");
        }
    }

    void Renderer::UpdateUniformBuffer(void* bufferMem, const Mat44& cameraMatrix)
    {
        GlobalUniformBufferObject ubo {};
        ubo.model = Mat44(kIdentityTag);
        ubo.view = Mat44::AffineInverse(cameraMatrix); 

        VkExtent2D extent = m_swapChain.GetExtent();
        const float aspectRatio = static_cast<float>(extent.width) / extent.height;
        ubo.projection = PerspectiveProjection(ToRadians(45.0f), aspectRatio, 0.1f, 100.0f);
        memcpy(bufferMem, &ubo, sizeof(ubo));
    }

    void Renderer::RenderFrameData::Init(VkDevice device, VkCommandPool commandPool)
    {
        // Create Command Buffer
        {
            VkCommandBufferAllocateInfo allocInfo {};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = commandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

            VkResult res = vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());

            SJ_ASSERT(res == VK_SUCCESS, "Failed to create graphics command buffer");
        }

        // Create Sync Primitives
        {
            VkSemaphoreCreateInfo semaphoreInfo {};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            VkFenceCreateInfo fenceInfo {};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            for(int i = 0; i < kMaxFramesInFlight; i++)
            {
                VkResult res = vkCreateSemaphore(device,
                                                 &semaphoreInfo,
                                                 sj::g_vkAllocationFns,
                                                 &imageAvailableSemaphores[i]);
                SJ_ASSERT(res == VK_SUCCESS, "Failed to create syncronization primitive");

                res = vkCreateSemaphore(device,
                                        &semaphoreInfo,
                                        sj::g_vkAllocationFns,
                                        &renderFinishedSemaphores[i]);
                SJ_ASSERT(res == VK_SUCCESS, "Failed to create syncronization primitive");

                res = vkCreateFence(device, &fenceInfo, sj::g_vkAllocationFns, &inFlightFences[i]);
                SJ_ASSERT(res == VK_SUCCESS, "Failed to create syncronization primitive");
            }
        }
    }

    void Renderer::RenderFrameData::DeInit(VkDevice device)
    {
        // NOTE: Command buffers are freed for us when we free the command pool.
        //       We only need to clean up sync primitives

        for(int i = 0; i < kMaxFramesInFlight; i++)
        {
            vkDestroySemaphore(device, imageAvailableSemaphores[i], sj::g_vkAllocationFns);
            vkDestroySemaphore(device, renderFinishedSemaphores[i], sj::g_vkAllocationFns);
            vkDestroyFence(device, inFlightFences[i], sj::g_vkAllocationFns);

            vkDestroyBuffer(device, globalUniformBuffers[i], sj::g_vkAllocationFns);
            vkFreeMemory(device, globalUniformBuffersMemory[i], sj::g_vkAllocationFns);
        }
    }


} // namespace sj
