// Screwjank Headers
#include <ScrewjankShared/utils/Assert.hpp>
#include <ScrewjankEngine/utils/Log.hpp>
#include <ScrewjankEngine/core/Window.hpp>
#include <ScrewjankEngine/rendering/Renderer.hpp>

// Shared Headers
#include <ScrewjankShared/Math/Helpers.hpp>
#include <ScrewjankShared/DataDefinitions/Texture.hpp>
#include <ScrewjankShared/io/File.hpp>

// Library Headers
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

// STD Headers

namespace sj
{
    static void CheckImguiVulkanResult(VkResult res)
    {
        SJ_ASSERT(res == VK_SUCCESS, "ImGui Vulkan operation failed!");
    }

    MemSpace<FreeListAllocator>* Renderer::WorkBuffer()
    {
        static MemSpace zone(MemorySystem::GetRootMemSpace(), k1_MiB * 2, "Renderer Work Buffer");
        return &zone;
    }

    Renderer* Renderer::GetInstance()
    {
        static Renderer s_renderer;
        return &s_renderer;
    }

    void Renderer::Render(const Mat44& cameraMatrix)
    {
        ImGui::Render();

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
        CreateDummyVertexBuffer();
        CreateDummyTexture();
        CreateDummyIndexBuffer();

        CreateGlobalUniformBuffers();

        CreateGlobalUBODescriptorPool();
        CreateImGuiDescriptorPool();

        CreateGlobalUBODescriptorSets();

        m_frameData.Init(m_renderDevice.GetLogicalDevice(), m_graphicsCommandPool);

        DeviceQueueFamilyIndices indices =
            VulkanRenderDevice::GetDeviceQueueFamilyIndices(m_renderDevice.GetPhysicalDevice());

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
    }

    void Renderer::CreateGlobalDescriptorSetlayout()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding {};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

        VkDescriptorSetLayoutCreateInfo layoutInfo {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        VkResult res = vkCreateDescriptorSetLayout(m_renderDevice.GetLogicalDevice(),
                                                   &layoutInfo,
                                                   nullptr,
                                                   &m_globalUBODescriptorSetLayout);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to create descriptor set layout");
    }


    void Renderer::DeInit()
    {
        VkDevice logicalDevice = m_renderDevice.GetLogicalDevice();
        vkDeviceWaitIdle(logicalDevice);

        ImGui_ImplVulkan_Shutdown();

        m_frameData.DeInit(logicalDevice);

        vkDestroyCommandPool(logicalDevice, m_graphicsCommandPool, nullptr);

        m_swapChain.DeInit(logicalDevice);

        vkDestroyDescriptorPool(logicalDevice, m_globalUBODescriptorPool, nullptr);
        vkDestroyDescriptorPool(logicalDevice, m_imguiDescriptorPool, nullptr);

        vkDestroyDescriptorSetLayout(logicalDevice, m_globalUBODescriptorSetLayout, nullptr);

        m_defaultPipeline.DeInit();

        vkDestroyRenderPass(logicalDevice, m_defaultRenderPass, nullptr);

        vkDestroyBuffer(logicalDevice, m_dummyVertexBuffer, nullptr);
        vkFreeMemory(logicalDevice, m_dummyVertexBufferMem, nullptr);

        vkDestroyBuffer(logicalDevice, m_dummyIndexBuffer, nullptr);
        vkFreeMemory(logicalDevice, m_dummyIndexBufferMem, nullptr);

        // Important: All things attached to the device need to be torn down first
        m_renderDevice.DeInit();

        vkDestroySurfaceKHR(m_vkInstance, m_renderingSurface, nullptr);

        if constexpr(g_IsDebugBuild)
        {
            auto messenger_destroy_func = (PFN_vkDestroyDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr(m_vkInstance, "vkDestroyDebugUtilsMessengerEXT");

            SJ_ASSERT(messenger_destroy_func != nullptr,
                      "Failed to load Vulkan Debug messenger destroy function");

            messenger_destroy_func(m_vkInstance, m_vkDebugMessenger, nullptr);
        }

        vkDestroyInstance(m_vkInstance, nullptr);
    }

    VkInstance Renderer::GetVkInstanceHandle() const
    {
        return m_vkInstance;
    }

    void Renderer::StartRenderFrame()
    {
        Viewport viewport = Window::GetInstance()->GetViewportSize();
        ImGui::GetIO().DisplaySize = {(float)viewport.Width, (float)viewport.Height};

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui::NewFrame();
    }

    uint32_t Renderer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_renderDevice.GetPhysicalDevice(), &memProperties);

        for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if((typeFilter & (1 << i)) &&
               (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        SJ_ASSERT(false, "Failed to find suitable memory type.");
        return -1;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::VulkanDebugLogCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
        void* user_data)
    {
        if(severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            SJ_ENGINE_LOG_TRACE("Vulkan Validation Layer message: {}", callback_data->pMessage);
        }
        else if(severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            SJ_ENGINE_LOG_INFO("Vulkan Validation Layer message: {}", callback_data->pMessage);
        }
        else if(severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            SJ_ENGINE_LOG_WARN("Vulkan Validation Layer message: {}", callback_data->pMessage);
        }
        else if(severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            SJ_ENGINE_LOG_ERROR("Vulkan Validation Layer message: {}", callback_data->pMessage);
        }

        return VK_FALSE;
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
        dynamic_vector<const char*> extenstions = GetRequiredExtenstions();
        create_info.enabledExtensionCount = (uint32_t)extenstions.size();
        create_info.ppEnabledExtensionNames = extenstions.data();

        // Compile-time check for adding validation layers
        if constexpr(g_IsDebugBuild)
        {
            static dynamic_vector<const char*> layers(MemorySystem::GetRootMemSpace(),
                                                      {"VK_LAYER_KHRONOS_validation"});

            EnableValidationLayers(layers);

            create_info.enabledLayerCount = (uint32_t)layers.size();
            create_info.ppEnabledLayerNames = layers.data();
        }

        // Create the vulkan instance
        VkResult result = vkCreateInstance(&create_info, nullptr, &m_vkInstance);
        SJ_ASSERT(result == VK_SUCCESS,
                  "Vulkan instance creation failed with error code {}",
                  result);

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

    dynamic_vector<const char*> Renderer::GetRequiredExtenstions() const
    {
        dynamic_vector<const char*> extensions_vector;

        extensions_vector = Window::GetInstance()->GetRequiredVulkanExtenstions();

        if constexpr(g_IsDebugBuild)
        {
            extensions_vector.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions_vector;
    }

    void Renderer::CreateRenderSurface()
    {
        m_renderingSurface = Window::GetInstance()->CreateWindowSurface(m_vkInstance);
    }

    void Renderer::CreateRenderPass()
    {
        VkAttachmentDescription colorAttachment {};
        colorAttachment.format = m_swapChain.GetImageFormat();
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

        VkResult res = vkCreateRenderPass(m_renderDevice.GetLogicalDevice(),
                                          &renderPassInfo,
                                          nullptr,
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
    Renderer::EnableValidationLayers(const dynamic_vector<const char*>& required_validation_layers)
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

        create_function(m_vkInstance, &messenger_create_info, nullptr, &m_vkDebugMessenger);
    }

    void Renderer::TransitionImageLayout(VkImage image,
                                         VkFormat format,
                                         VkImageLayout oldLayout,
                                         VkImageLayout newLayout,
                                         uint32_t srcQueueIdx,
                                         uint32_t dstQueueIdx)
    {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

        VkImageMemoryBarrier barrier {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = srcQueueIdx;
        barrier.dstAccessMask = dstQueueIdx;

        //asdfasdf

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
                                               nullptr,
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

        VkResult res = vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &out_buffer);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to create vulkan buffer");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(logicalDevice, out_buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

        res = vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &out_bufferMemory);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to allocate dummy vertex buffer memory");

        vkBindBufferMemory(logicalDevice, out_buffer, out_bufferMemory, 0);
    }

    void
    Renderer::CreateImage(const VkImageCreateInfo& imageInfo, VkImage& image, VkDeviceMemory& imageMem)
    {
        VkResult res = vkCreateImage(m_renderDevice.GetLogicalDevice(),
                                     &imageInfo,
                                     nullptr,
                                     &m_dummyTextureImage);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to create dummy texture image");

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_renderDevice.GetLogicalDevice(),
                                     m_dummyTextureImage,
                                     &memRequirements);

        VkMemoryAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex =
            FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        res = vkAllocateMemory(m_renderDevice.GetLogicalDevice(),
                               &allocInfo,
                               nullptr,
                               &m_dummyTextureImageMemory);
        SJ_ASSERT(res == VK_SUCCESS, "Failed to allocate dummy texture memory on GPU");

        vkBindImageMemory(m_renderDevice.GetLogicalDevice(),
                          m_dummyTextureImage,
                          m_dummyTextureImageMemory,
                          0);
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

    void Renderer::CreateDummyVertexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(m_dummyVertices[0]) * m_dummyVertices.size();

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

        memcpy(data, m_dummyVertices.data(), static_cast<size_t>(bufferSize));

        vkUnmapMemory(m_renderDevice.GetLogicalDevice(), stagingBufferMemory);

        CreateBuffer(bufferSize,
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     m_dummyVertexBuffer,
                     m_dummyVertexBufferMem);

        CopyBuffer(stagingBuffer, m_dummyVertexBuffer, bufferSize);

        vkDestroyBuffer(m_renderDevice.GetLogicalDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_renderDevice.GetLogicalDevice(), stagingBufferMemory, nullptr);
    }

    void Renderer::CreateDummyIndexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(m_dummyIndices[0]) * m_dummyIndices.size();

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

        memcpy(data, m_dummyIndices.data(), (size_t)bufferSize);
        vkUnmapMemory(m_renderDevice.GetLogicalDevice(), stagingBufferMemory);

        CreateBuffer(bufferSize,
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     m_dummyIndexBuffer,
                     m_dummyIndexBufferMem);

        CopyBuffer(stagingBuffer, m_dummyIndexBuffer, bufferSize);

        vkDestroyBuffer(m_renderDevice.GetLogicalDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_renderDevice.GetLogicalDevice(), stagingBufferMemory, nullptr);
    }

    void Renderer::CreateDummyTexture()
    {
        File textureFile;
        textureFile.Open("Data/Engine/texture.sj_tex", sj::File::OpenMode::kReadBinary);
        uint64_t bytes = textureFile.Size();
        MemSpaceScope scope(WorkBuffer());
        void* textureMem = scope->Allocate(bytes, alignof(Texture));
        textureFile.Read(textureMem, bytes);
        Texture* texture = reinterpret_cast<Texture*>(textureMem);

        VkDeviceSize imageSize = texture->width * texture->height * 4;
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        CreateBuffer(imageSize,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     stagingBuffer,
                     stagingBufferMemory);

        void* data;
        vkMapMemory(m_renderDevice.GetLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, texture->data, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_renderDevice.GetLogicalDevice(), stagingBufferMemory);

        VkImageCreateInfo imageInfo {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = texture->width;
        imageInfo.extent.height = texture->height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; // Optional
        
        scope->Free(textureMem);
        textureFile.Close();
        CreateImage(imageInfo, m_dummyTextureImage, m_dummyTextureImageMemory);
        
        return;
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
        VkDescriptorPoolSize poolSize {};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = kMaxFramesInFlight;

        VkDescriptorPoolCreateInfo poolInfo {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = kMaxFramesInFlight;

        VkResult res = vkCreateDescriptorPool(m_renderDevice.GetLogicalDevice(),
                                              &poolInfo,
                                              nullptr,
                                              &m_globalUBODescriptorPool);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to create descriptor pool for global UBO");
    }

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
                                              nullptr,
                                              &m_imguiDescriptorPool);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to create descriptor pool for global UBO");
    }

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

            VkWriteDescriptorSet descriptorWrite {};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = m_frameData.globalUBODescriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(logicalDevice, 1, &descriptorWrite, 0, nullptr);
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

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;
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

            vkCmdDrawIndexed(buffer, static_cast<uint32_t>(m_dummyIndices.size()), 1, 0, 0, 0);

            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), buffer);
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
        ubo.view = AffineInverse(cameraMatrix); 

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
                                                 nullptr,
                                                 &imageAvailableSemaphores[i]);
                SJ_ASSERT(res == VK_SUCCESS, "Failed to create syncronization primitive");

                res = vkCreateSemaphore(device,
                                        &semaphoreInfo,
                                        nullptr,
                                        &renderFinishedSemaphores[i]);
                SJ_ASSERT(res == VK_SUCCESS, "Failed to create syncronization primitive");

                res = vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]);
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
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);

            vkDestroyBuffer(device, globalUniformBuffers[i], nullptr);
            vkFreeMemory(device, globalUniformBuffersMemory[i], nullptr);
        }
    }


} // namespace sj
