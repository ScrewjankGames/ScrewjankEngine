// STD Headers
#include <unordered_set>

// Library Headers

// Screwjank Headers
#include <ScrewjankEngine/platform/Vulkan/VulkanRendererAPI.hpp>

#include <ScrewjankEngine/core/Window.hpp>
#include <ScrewjankEngine/containers/String.hpp>
#include <ScrewjankEngine/containers/UnorderedSet.hpp>
#include <ScrewjankEngine/rendering/Renderer.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanRenderDevice.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanSwapChain.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanPipeline.hpp>

#ifdef SJ_PLATFORM_WINDOWS
#include <ScrewjankEngine/platform/Windows/WindowsWindow.hpp>
#endif // SJ_PLATFORM_WINDOWS


namespace sj
{
    VulkanRendererAPI::VulkanRendererAPI() : m_SwapChainBuffers(Renderer::WorkBuffer())
    {

    }

    VulkanRendererAPI::~VulkanRendererAPI()
    {
        DeInit();
    }

    void VulkanRendererAPI::Init()
    {
        SJ_ASSERT(!m_IsInitialized, "Double initialization of Vulkan detected");
        InitializeVulkan();
        m_IsInitialized = true;

        // Create rendering surface
        CreateRenderSurface();

        // Select physical device and create and logical render device
        m_RenderDevice.Init(m_RenderingSurface);

        // Create the vulkan swap chain connected to the current window and device
        m_SwapChain.Init(Window::GetInstance(),
                         m_RenderDevice.GetPhysicalDevice(),
                         m_RenderDevice.GetLogicalDevice(),
                         m_RenderingSurface);

        CreateRenderPass();

        m_DefaultPipeline.Init(
            m_RenderDevice.GetLogicalDevice(),
            m_SwapChain.GetExtent(),
            m_DefaultRenderPass,
            "Data/Engine/Shaders/Default.vert.spv",
            "Data/Engine/Shaders/Default.frag.spv"
        );

        CreateFrameBuffers();

        CreateCommandPool();

        CreateCommandBuffer();

        CreateSyncPrimitives();
    }

    void VulkanRendererAPI::DeInit()
    {
        if(!m_IsInitialized)
        {
            return;
        }

        if constexpr(g_IsDebugBuild)
        {
            auto messenger_destroy_func = (PFN_vkDestroyDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr(m_VkInstance, "vkDestroyDebugUtilsMessengerEXT");

            SJ_ASSERT(messenger_destroy_func != nullptr,
                      "Failed to load Vulkan Debug messenger destroy function");

            messenger_destroy_func(m_VkInstance, m_VkDebugMessenger, nullptr);
        }

        vkDeviceWaitIdle(m_RenderDevice.GetLogicalDevice());

        vkDestroySemaphore(m_RenderDevice.GetLogicalDevice(), m_ImageAvailableSemaphore, nullptr);
        vkDestroySemaphore(m_RenderDevice.GetLogicalDevice(), m_RenderFinishedSemaphore, nullptr);
        vkDestroyFence(m_RenderDevice.GetLogicalDevice(), m_InFlightFence, nullptr);


        vkDestroyCommandPool(m_RenderDevice.GetLogicalDevice(), m_CommandPool, nullptr);

        for(VkFramebuffer framebuffer : m_SwapChainBuffers)
        {
            vkDestroyFramebuffer(m_RenderDevice.GetLogicalDevice(), framebuffer, nullptr);
        }

        // Important: All things attached to the device need to be torn down first
        m_SwapChain.DeInit();
        m_DefaultPipeline.DeInit();
        vkDestroyRenderPass(m_RenderDevice.GetLogicalDevice(), m_DefaultRenderPass, nullptr);
        m_RenderDevice.DeInit();

        vkDestroySurfaceKHR(m_VkInstance, m_RenderingSurface, nullptr);
        vkDestroyInstance(m_VkInstance, nullptr);
    }

    void VulkanRendererAPI::InitializeVulkan()
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
        dynamic_vector<const char*> extenstions = GetRequiredExtenstions();
        create_info.enabledExtensionCount = (uint32_t)extenstions.size();
        create_info.ppEnabledExtensionNames = extenstions.Data();

        // Compile-time check for adding validation layers
        if constexpr (g_IsDebugBuild) 
        {
            static dynamic_vector<const char*> layers( 
                MemorySystem::GetRootHeapZone(),
                {"VK_LAYER_KHRONOS_validation"}
            );

            EnableValidationLayers(layers);

            create_info.enabledLayerCount = (uint32_t)layers.size();
            create_info.ppEnabledLayerNames = layers.Data();
        }


        // Create the vulkan instance
        VkResult result = vkCreateInstance(&create_info, nullptr, &m_VkInstance);
        SJ_ASSERT(result == VK_SUCCESS,
                  "Vulkan instance creation failed with error code {}",
                  result);

        // Compile-time check to enable debug messaging
        if constexpr (g_IsDebugBuild)
        {
            EnableDebugMessaging();
        }

        // Log success
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        SJ_ENGINE_LOG_INFO("Vulkan loaded with {} extensions supported", extensionCount);
    }

    static VulkanRendererAPI g_api;
    VulkanRendererAPI* VulkanRendererAPI::GetInstance()
    {
        return &g_api;
    }

    VkInstance VulkanRendererAPI::GetVkInstanceHandle() const
    {
        SJ_ASSERT(m_IsInitialized, "Attempting to access uninitialized VkInstance");
        return m_VkInstance;
    }

    void VulkanRendererAPI::DrawFrame()
    {
        vkWaitForFences(m_RenderDevice.GetLogicalDevice(),
                        1,
                        &m_InFlightFence,
                        VK_TRUE,
                        std::numeric_limits<uint64_t>::max());

        SJ_ENGINE_LOG_INFO("Kicking render frame");

        vkResetFences(m_RenderDevice.GetLogicalDevice(), 1, &m_InFlightFence);

        uint32_t imageIndex;
        vkAcquireNextImageKHR(m_RenderDevice.GetLogicalDevice(),
                              m_SwapChain.GetSwapChain(),
                              std::numeric_limits<uint64_t>::max(),
                              m_ImageAvailableSemaphore,
                              VK_NULL_HANDLE,
                              &imageIndex);
        
        vkResetCommandBuffer(m_CommandBuffer, 0);

        RecordCommandBuffer(m_CommandBuffer, imageIndex);

        VkSubmitInfo submitInfo {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {m_ImageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_CommandBuffer;

        VkSemaphore signalSemaphores[] = {m_RenderFinishedSemaphore};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        VkResult res =
            vkQueueSubmit(m_RenderDevice.GetGraphicsQueue(), 1, &submitInfo, m_InFlightFence);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to submit draw command buffer!");

        VkPresentInfoKHR presentInfo {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {m_SwapChain.GetSwapChain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(m_RenderDevice.GetPresentationQueue(), &presentInfo);

    }

    dynamic_vector<const char*> VulkanRendererAPI::GetRequiredExtenstions() const
    {
        dynamic_vector<const char*> extensions_vector;

        extensions_vector = Window::GetInstance()->GetRequiredVulkanExtenstions();

        if constexpr (g_IsDebugBuild)
        {
            extensions_vector.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions_vector;
    }

    void VulkanRendererAPI::CreateRenderSurface()
    {
        m_RenderingSurface = Window::GetInstance()->CreateWindowSurface(m_VkInstance);
    }

    void VulkanRendererAPI::CreateRenderPass()
    {
        VkAttachmentDescription colorAttachment {};
        colorAttachment.format = m_SwapChain.GetImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VkResult res = vkCreateRenderPass(m_RenderDevice.GetLogicalDevice(),
                                          &renderPassInfo,
                                          nullptr,
                                          &m_DefaultRenderPass);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to create render pass.");
    }

    void
    VulkanRendererAPI::EnableValidationLayers(const dynamic_vector<const char*>& required_validation_layers)
    {
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        SJ_ASSERT(layer_count <= 64, "Overflow");
        Array<VkLayerProperties, 64> available_layers;
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.Data());

        // Verify required validation layers are supported
        for (auto layer_name : required_validation_layers) {
            bool layer_found = false;

            for (const auto& layer_properties : available_layers) {
                if (strcmp(layer_name, layer_properties.layerName) == 0) {
                    layer_found = true;
                    break;
                }
            }

            if (!layer_found) {
                SJ_ASSERT(false, "Failed to enable vulkan validation layer {}", layer_name);
            }

            SJ_ENGINE_LOG_DEBUG("Enabled Vulkan validation layer: {}", layer_name);
        }
    }

    void VulkanRendererAPI::EnableDebugMessaging()
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
            m_VkInstance,
            "vkCreateDebugUtilsMessengerEXT");

        SJ_ASSERT(create_function != nullptr,
                  "Failed to load vulkan extension function vkCreateDebugUtilsMessengerEXT");

        create_function(m_VkInstance, &messenger_create_info, nullptr, &m_VkDebugMessenger);
    }

    void VulkanRendererAPI::CreateFrameBuffers()
    {
        std::span<VkImageView> imageViews = m_SwapChain.GetImageViews();
        m_SwapChainBuffers.resize(imageViews.size());

        int i = 0;
        for(VkImageView& view : imageViews)
        {

            VkFramebufferCreateInfo framebufferInfo {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_DefaultRenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &view;
            framebufferInfo.width = m_SwapChain.GetExtent().width;
            framebufferInfo.height = m_SwapChain.GetExtent().height;
            framebufferInfo.layers = 1;

            VkResult res = vkCreateFramebuffer(m_RenderDevice.GetLogicalDevice(),
                                               &framebufferInfo,
                                               nullptr,
                                               &m_SwapChainBuffers[i]);

            SJ_ASSERT(res == VK_SUCCESS, "Failed to construct frame buffers.");

            i++;
        }
    }

    void VulkanRendererAPI::CreateCommandPool()
    {
        auto indices =
            m_RenderDevice.GetDeviceQueueFamilyIndices(m_RenderDevice.GetPhysicalDevice());
        
        VkCommandPoolCreateInfo poolInfo {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = *(indices.GraphicsFamilyIndex);

        VkResult res = vkCreateCommandPool(m_RenderDevice.GetLogicalDevice(),
                                           &poolInfo,
                                           nullptr,
                                           &m_CommandPool);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to create graphics command pool");

    }

    void VulkanRendererAPI::CreateCommandBuffer()
    {
        VkCommandBufferAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_CommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkResult res = vkAllocateCommandBuffers(m_RenderDevice.GetLogicalDevice(),
                                                &allocInfo,
                                                &m_CommandBuffer);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to create graphics command buffer");
    }

    void VulkanRendererAPI::CreateSyncPrimitives()
    {
        VkSemaphoreCreateInfo semaphoreInfo {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        
        VkFenceCreateInfo fenceInfo {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkResult res = vkCreateSemaphore(m_RenderDevice.GetLogicalDevice(),
                                         &semaphoreInfo,
                                         nullptr,
                                         &m_ImageAvailableSemaphore);
        SJ_ASSERT(res == VK_SUCCESS, "Failed to create syncronization primitive");

        res = vkCreateSemaphore(m_RenderDevice.GetLogicalDevice(),
                                &semaphoreInfo,
                                nullptr,
                                &m_RenderFinishedSemaphore);
        SJ_ASSERT(res == VK_SUCCESS, "Failed to create syncronization primitive");

        res = vkCreateFence(m_RenderDevice.GetLogicalDevice(),
                            &fenceInfo,
                            nullptr,
                            &m_InFlightFence);
        SJ_ASSERT(res == VK_SUCCESS, "Failed to create syncronization primitive");


    }

    void VulkanRendererAPI::RecordCommandBuffer(VkCommandBuffer buffer, uint32_t imageIdx)
    {
        VkCommandBufferBeginInfo beginInfo {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        {
            VkResult res = vkBeginCommandBuffer(m_CommandBuffer, &beginInfo);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to create command buffer");
        }

        VkRenderPassBeginInfo renderPassInfo {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_DefaultRenderPass;
        renderPassInfo.framebuffer = m_SwapChainBuffers[imageIdx];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_SwapChain.GetExtent();

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;
        vkCmdBeginRenderPass(m_CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            vkCmdBindPipeline(m_CommandBuffer,
                              VK_PIPELINE_BIND_POINT_GRAPHICS,
                              m_DefaultPipeline.GetPipeline());
   
            VkViewport viewport {};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(m_SwapChain.GetExtent().width);
            viewport.height = static_cast<float>(m_SwapChain.GetExtent().height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(m_CommandBuffer, 0, 1, &viewport);

            VkRect2D scissor {};
            scissor.offset = {0, 0};
            scissor.extent = m_SwapChain.GetExtent();
            vkCmdSetScissor(m_CommandBuffer, 0, 1, &scissor);

            vkCmdDraw(m_CommandBuffer, 3, 1, 0, 0);
        }
        vkCmdEndRenderPass(m_CommandBuffer);

        {
            VkResult res = vkEndCommandBuffer(m_CommandBuffer);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to record command buffer!");
        }
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRendererAPI::VulkanDebugLogCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
        void* user_data)
    {
        if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
            SJ_ENGINE_LOG_TRACE("Vulkan Validation Layer message: {}", callback_data->pMessage);
        } else if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            SJ_ENGINE_LOG_INFO("Vulkan Validation Layer message: {}", callback_data->pMessage);
        } else if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            SJ_ENGINE_LOG_WARN("Vulkan Validation Layer message: {}", callback_data->pMessage);
        } else if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            SJ_ENGINE_LOG_ERROR("Vulkan Validation Layer message: {}", callback_data->pMessage);
        }

        return VK_FALSE;
    }
} // namespace sj
