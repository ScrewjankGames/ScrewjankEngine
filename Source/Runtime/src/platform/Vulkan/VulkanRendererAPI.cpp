// Parent Include
#include <ScrewjankEngine/platform/Vulkan/VulkanRendererAPI.hpp>

// STD Headers
#include <unordered_set>

// Library Headers

// Screwjank Headers
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
        m_renderDevice.Init(m_renderingSurface);

        // Create the vulkan swap chain connected to the current window and device
        m_swapChain.Init(m_renderDevice.GetPhysicalDevice(),
                         m_renderDevice.GetLogicalDevice(),
                         m_renderingSurface);

        CreateRenderPass();

        m_defaultPipeline.Init(
            m_renderDevice.GetLogicalDevice(),
            m_swapChain.GetExtent(),
            m_defaultRenderPass,
            "Data/Engine/Shaders/Default.vert.spv",
            "Data/Engine/Shaders/Default.frag.spv"
        );

        m_swapChain.InitFrameBuffers(m_renderDevice.GetLogicalDevice(), m_defaultRenderPass);

        CreateCommandPools();
        CreateDummyVertexBuffer();
        CreateDummyIndexBuffer();

        m_frameData.Init(m_renderDevice.GetLogicalDevice(), m_graphicsCommandPool);
    }

    void VulkanRendererAPI::DeInit()
    {
        if(!m_IsInitialized)
        {
            return;
        }

        vkDeviceWaitIdle(m_renderDevice.GetLogicalDevice());

        vkDestroyBuffer(m_renderDevice.GetLogicalDevice(), m_dummyVertexBuffer, nullptr);
        vkFreeMemory(m_renderDevice.GetLogicalDevice(), m_dummyVertexBufferMem, nullptr);

        m_frameData.DeInit(m_renderDevice.GetLogicalDevice());

        vkDestroyCommandPool(m_renderDevice.GetLogicalDevice(), m_graphicsCommandPool, nullptr);
        vkDestroyCommandPool(m_renderDevice.GetLogicalDevice(), m_transferCommandPool, nullptr);

        m_swapChain.DeInit(m_renderDevice.GetLogicalDevice());
        
        m_defaultPipeline.DeInit();
        
        vkDestroyRenderPass(m_renderDevice.GetLogicalDevice(), m_defaultRenderPass, nullptr);
        
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
        create_info.ppEnabledExtensionNames = extenstions.data();

        // Compile-time check for adding validation layers
        if constexpr (g_IsDebugBuild) 
        {
            static dynamic_vector<const char*> layers( 
                MemorySystem::GetRootHeapZone(),
                {"VK_LAYER_KHRONOS_validation"}
            );

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
        return m_vkInstance;
    }

    void VulkanRendererAPI::DrawFrame()
    {
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

        // Reset fence when we know we're going to be able to draw this frame
        vkResetFences(m_renderDevice.GetLogicalDevice(), 1, &currFence);
        
        vkResetCommandBuffer(currCommandBuffer, 0);

        RecordCommandBuffer(currCommandBuffer, imageIndex);

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

    uint32_t VulkanRendererAPI::FindMemoryType(uint32_t typeFilter,
                                               VkMemoryPropertyFlags properties)
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
        m_renderingSurface = Window::GetInstance()->CreateWindowSurface(m_vkInstance);
    }

    void VulkanRendererAPI::CreateRenderPass()
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

    void
    VulkanRendererAPI::EnableValidationLayers(const dynamic_vector<const char*>& required_validation_layers)
    {
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        SJ_ASSERT(layer_count <= 64, "Overflow");
        array<VkLayerProperties, 64> available_layers;
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

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
            m_vkInstance,
            "vkCreateDebugUtilsMessengerEXT");

        SJ_ASSERT(create_function != nullptr,
                  "Failed to load vulkan extension function vkCreateDebugUtilsMessengerEXT");

        create_function(m_vkInstance, &messenger_create_info, nullptr, &m_vkDebugMessenger);
    }

    void VulkanRendererAPI::CreateCommandPools()
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

        // Create memory transfer command pool
        {
            VkCommandPoolCreateInfo poolInfo {};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolInfo.queueFamilyIndex = *(indices.transferFamilyIndex);
            
            VkResult res = vkCreateCommandPool(m_renderDevice.GetLogicalDevice(),
                                               &poolInfo,
                                               nullptr,
                                               &m_transferCommandPool);

            SJ_ASSERT(res == VK_SUCCESS, "Failed to create graphics command pool");
        }

    }

    void VulkanRendererAPI::CreateBuffer(VkDeviceSize size,
                                         VkBufferUsageFlags usage,
                                         VkMemoryPropertyFlags properties,
                                         VkBuffer& out_buffer,
                                         VkDeviceMemory& out_bufferMemory)
    {
        VkDevice logicalDevice = m_renderDevice.GetLogicalDevice();

        DeviceQueueFamilyIndices indices =
            VulkanRenderDevice::GetDeviceQueueFamilyIndices(m_renderDevice.GetPhysicalDevice());

        array<uint32_t, 2> queueFamilyIndices {*indices.graphicsFamilyIndex,
                                               *indices.transferFamilyIndex};


        VkBufferCreateInfo bufferInfo {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        bufferInfo.queueFamilyIndexCount = 2;
        bufferInfo.pQueueFamilyIndices = queueFamilyIndices.data();

        VkResult res = vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &out_buffer);
        
        SJ_ASSERT(res == VK_SUCCESS, "Failed to create vulkan buffer");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(logicalDevice,
                                      out_buffer,
                                      &memRequirements);

        VkMemoryAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

        res = vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &out_bufferMemory);

        SJ_ASSERT(res == VK_SUCCESS, "Failed to allocate dummy vertex buffer memory");

        vkBindBufferMemory(logicalDevice, out_buffer, out_bufferMemory, 0);
    }

    void VulkanRendererAPI::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        VkCommandBufferAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_transferCommandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_renderDevice.GetLogicalDevice(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkBufferCopy copyRegion {};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(m_renderDevice.GetTransferQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_renderDevice.GetTransferQueue());

        vkFreeCommandBuffers(
            m_renderDevice.GetLogicalDevice(), 
            m_transferCommandPool, 
            1, 
            &commandBuffer);
    }

    void VulkanRendererAPI::CreateDummyVertexBuffer()
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

    void VulkanRendererAPI::CreateDummyIndexBuffer()
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

    void VulkanRendererAPI::RecordCommandBuffer(VkCommandBuffer buffer, uint32_t imageIdx)
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

            vkCmdDrawIndexed(buffer, static_cast<uint32_t>(m_dummyIndices.size()), 1, 0, 0, 0);
        }
        vkCmdEndRenderPass(buffer);

        {
            VkResult res = vkEndCommandBuffer(buffer);
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

    void VulkanRendererAPI::RenderFrameData::Init(VkDevice device, VkCommandPool commandPool)
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

                res = vkCreateFence(device,
                                    &fenceInfo,
                                    nullptr, 
                                    &inFlightFences[i]);
                SJ_ASSERT(res == VK_SUCCESS, "Failed to create syncronization primitive");
            }
        }
    }

    void VulkanRendererAPI::RenderFrameData::DeInit(VkDevice device)
    {
        // NOTE: Command buffers are freed for us when we free the command pool.
        //       We only need to clean up sync primitives

        for(int i = 0; i < kMaxFramesInFlight; i++)
        {
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }
    }

} // namespace sj
