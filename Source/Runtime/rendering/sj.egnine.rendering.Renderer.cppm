module;

// Screwjank Headers
#include <ScrewjankEngine/framework/Window.hpp>

#include <ScrewjankDataDefinitions/Assets/Model.hpp>

#include <ScrewjankStd/Log.hpp>
#include <ScrewjankStd/Assert.hpp>

// Library Headers
#include <vulkan/vulkan_core.h>
#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

#ifndef SJ_GOLD
    #include <imgui_impl_glfw.h>
    #include <imgui_impl_vulkan.h>
#endif

// STD Headers
#include <array>
#include <cstring>
#include <cstddef>
#include <fstream>

export module sj.engine.rendering.Renderer;
import sj.engine.rendering.resources.TextureResource;
import sj.engine.rendering.vk;
import sj.engine.system.threading.ThreadContext;
import sj.engine.system.memory.MemorySystem;
import sj.datadefs.assets.Texture;
import sj.std.containers.vector;
import sj.std.math;
import sj.std.memory.literals;
import sj.std.memory.resources.free_list_allocator;

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

        Renderer() : m_swapChain(WorkBuffer())
        {
        }

        ~Renderer() = default;

        void Init()
        {
            Window* window = Window::GetInstance();

            free_list_allocator* workBuffer = WorkBuffer();
            workBuffer->init(4_MiB, *MemorySystem::GetRootMemoryResource());
            MemorySystem::TrackMemoryResource(workBuffer);

            vkb::Instance bootstrapInfo = InitializeVulkan();

            m_renderingSurface = window->CreateWindowSurface(m_vkInstance);

            // Select physical device and create and logical render device
            m_renderDevice.Init(bootstrapInfo, m_renderingSurface);

            // Create the vulkan swap chain connected to the current window and device
            m_swapChain.Init(m_renderDevice, m_renderingSurface, window);
            CreateRenderTarget(window->GetViewportSize());

            CreateGlobalDescriptorSetlayout();

            m_defaultPipeline.Init(m_renderDevice.GetLogicalDevice(),
                                   m_globalUBODescriptorSetLayout,
                                   "Data/Engine/Shaders/Default.vert.spv",
                                   "Data/Engine/Shaders/Default.frag.spv");

            CreateCommandPools();

            LoadDummyModel();
            CreateDummyTextureImage();

            m_dummyTextureImage.imageView = CreateImageView(m_renderDevice.GetLogicalDevice(),
                                                            m_dummyTextureImage.image,
                                                            m_dummyTextureImage.imageFormat,
                                                            VK_IMAGE_ASPECT_COLOR_BIT);

            CreateDummyTextureSampler();

            CreateGlobalUniformBuffers();

            CreateGlobalUBODescriptorPool();

#ifndef SJ_GOLD
            CreateImGuiDescriptorPool();
#endif // !SJ_GOLD

            CreateGlobalUBODescriptorSets();

            VkFenceCreateInfo fenceInfo {};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            vkCreateFence(m_renderDevice.GetLogicalDevice(),
                          &fenceInfo,
                          sj::g_vkAllocationFns,
                          &m_immediateFence);

            m_frameData.Init(m_renderDevice.GetLogicalDevice(), m_graphicsCommandPool);

#ifndef SJ_GOLD
            // Init ImGui
            ImGui_ImplVulkan_InitInfo init_info = {};
            init_info.Instance = m_vkInstance;
            init_info.PhysicalDevice = m_renderDevice.GetPhysicalDevice();
            init_info.Device = m_renderDevice.GetLogicalDevice();
            init_info.QueueFamily = m_renderDevice.GetGraphicsQueueIndex();
            init_info.Queue = m_renderDevice.GetGraphicsQueue();
            init_info.PipelineCache = VK_NULL_HANDLE;
            init_info.DescriptorPool = m_imguiDescriptorAllocator.GetPool();
            init_info.Allocator = nullptr;
            init_info.MinImageCount = kMaxFramesInFlight;
            init_info.ImageCount = kMaxFramesInFlight;
            init_info.CheckVkResultFn = CheckImguiVulkanResult;

            VkFormat swapchainImageFormat = m_swapChain.GetImageFormat();
            init_info.UseDynamicRendering = true;
            init_info.PipelineRenderingCreateInfo.sType =
                VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
            init_info.PipelineRenderingCreateInfo.pNext = nullptr;
            init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
            init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swapchainImageFormat;

            init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
            ImGui_ImplVulkan_Init(&init_info);
#endif
        }

        void DeInit()
        {
            VkDevice logicalDevice = m_renderDevice.GetLogicalDevice();
            vkDeviceWaitIdle(logicalDevice);

#ifndef SJ_GOLD
            ImGui_ImplVulkan_Shutdown();
#endif // !SJ_GOLD

            vkDestroyFence(m_renderDevice.GetLogicalDevice(),
                           m_immediateFence,
                           sj::g_vkAllocationFns);

            m_frameData.DeInit(m_renderDevice);

            vkDestroyCommandPool(logicalDevice, m_graphicsCommandPool, sj::g_vkAllocationFns);
            vkDestroyCommandPool(logicalDevice, m_immediateCommandPool, sj::g_vkAllocationFns);

            DestroyRenderTarget();
            m_swapChain.DeInit(m_renderDevice);

            vkDestroySampler(logicalDevice, m_dummyTextureSampler, sj::g_vkAllocationFns);
            vkDestroyImageView(logicalDevice, m_dummyTextureImage.imageView, sj::g_vkAllocationFns);
            vmaDestroyImage(m_renderDevice.GetAllocator(),
                            m_dummyTextureImage.image,
                            m_dummyTextureImage.allocation);

            vkDestroyDescriptorPool(logicalDevice,
                                    m_globalUBODescriptorPool,
                                    sj::g_vkAllocationFns);

            if constexpr(g_IsDebugBuild)
            {
                m_imguiDescriptorAllocator.DeInit(logicalDevice);
            }

            vkDestroyDescriptorSetLayout(logicalDevice,
                                         m_globalUBODescriptorSetLayout,
                                         sj::g_vkAllocationFns);

            m_defaultPipeline.DeInit(logicalDevice);

            vmaDestroyBuffer(m_renderDevice.GetAllocator(),
                             m_dummyVertexBuffer.buffer,
                             m_dummyVertexBuffer.allocation);

            vmaDestroyBuffer(m_renderDevice.GetAllocator(),
                             m_dummyIndexBuffer.buffer,
                             m_dummyIndexBuffer.allocation);

            // Important: All things attached to the device need to be torn down first
            m_renderDevice.DeInit();

            vkDestroySurfaceKHR(m_vkInstance, m_renderingSurface, sj::g_vkAllocationFns);

            if constexpr(g_IsDebugBuild)
            {
                auto messenger_destroy_func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                    vkGetInstanceProcAddr(m_vkInstance, "vkDestroyDebugUtilsMessengerEXT"));

                SJ_ASSERT(messenger_destroy_func != nullptr,
                          "Failed to load Vulkan Debug messenger destroy function");

                messenger_destroy_func(m_vkInstance, m_vkDebugMessenger, sj::g_vkAllocationFns);
            }

            vkDestroyInstance(m_vkInstance, sj::g_vkAllocationFns);
        }

        void StartRenderFrame()
        {
            Viewport viewport = Window::GetInstance()->GetViewportSize();

#ifndef SJ_GOLD
            {
                MemoryResourceScope _(MemorySystem::GetDebugMemoryResource());

                ImGui::GetIO().DisplaySize = {(float)viewport.Width, (float)viewport.Height};

                // Start the Dear ImGui frame
                ImGui_ImplVulkan_NewFrame();
                ImGui::NewFrame();
            }
#endif // !SJ_GOLD
        }

        void Render(const Mat44& cameraMatrix)
        {
#ifndef SJ_GOLD
            ImGui::Render();
#endif // !SJ_GOLD

            const uint32_t frameIdx = m_frameCount % kMaxFramesInFlight;

            VkCommandBuffer currCommandBuffer = m_frameData.commandBuffers[frameIdx];
            VkFence currFence = m_frameData.inFlightFences[frameIdx];
            VkSemaphore currImageAvailableSemaphore =
                m_frameData.imageAvailableSemaphores[frameIdx];

            vkWaitForFences(m_renderDevice.GetLogicalDevice(),
                            1,
                            &currFence,
                            VK_TRUE,
                            std::numeric_limits<uint64_t>::max());

            VkSwapchainKHR swapChain = m_swapChain.GetSwapChain();
            uint32_t imageIndex = 0;
            VkResult res = vkAcquireNextImageKHR(m_renderDevice.GetLogicalDevice(),
                                                 swapChain,
                                                 std::numeric_limits<uint64_t>::max(),
                                                 currImageAvailableSemaphore,
                                                 VK_NULL_HANDLE,
                                                 &imageIndex);

            VkSemaphore currRenderFinishedSemaphore =
                m_swapChain.GetImageRenderCompleteSemaphore(imageIndex);

            if(res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
            {
                m_swapChain.Recreate(m_renderDevice, m_renderingSurface, Window::GetInstance());

                return;
            }
            else if(res != VK_SUCCESS)
            {
                SJ_ASSERT(false, "Failed to acquire swap chain image.");
            }

            // Reset fence when we know we're going to be able to draw this frame
            vkResetFences(m_renderDevice.GetLogicalDevice(), 1, &currFence);

            vkResetCommandBuffer(currCommandBuffer, 0);

            UpdateUniformBuffer(m_frameData.globalUniformBuffers[frameIdx].info.pMappedData,
                                cameraMatrix);

            RecordDrawCommands(currCommandBuffer, frameIdx, imageIndex);

            VkCommandBufferSubmitInfo submitCommandInfo =
                sj::vk::MakeCommandBufferSubmitInfo(currCommandBuffer);

            VkSemaphoreSubmitInfo waitInfo =
                sj::vk::MakeSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
                                                currImageAvailableSemaphore);

            VkSemaphoreSubmitInfo signalInfo =
                sj::vk::MakeSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
                                                currRenderFinishedSemaphore);

            VkSubmitInfo2 submitInfo =
                sj::vk::MakeSubmitInfo(&submitCommandInfo, &signalInfo, &waitInfo);

            res = vkQueueSubmit2(m_renderDevice.GetGraphicsQueue(), 1, &submitInfo, currFence);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to submit draw command buffer!");

            VkPresentInfoKHR presentInfo {};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = &currRenderFinishedSemaphore;
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = &swapChain;
            presentInfo.pImageIndices = &imageIndex;

            res = vkQueuePresentKHR(m_renderDevice.GetPresentationQueue(), &presentInfo);
            m_frameCount++;
            if(res == VK_ERROR_OUT_OF_DATE_KHR)
            {
                m_swapChain.Recreate(m_renderDevice, m_renderingSurface, Window::GetInstance());
                return;
            }
            else if(res != VK_SUCCESS)
            {
                SJ_ASSERT(false, "Failed to acquire swap chain image.");
            }
        }

        sj::vk::RenderDevice* GetRenderDevice()
        {
            return &m_renderDevice;
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
        static constexpr uint32_t kMaxFramesInFlight = 2;

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

        static bool HasStencilComponent(VkFormat format);

        /**
         * Initializes the Vulkan API's instance and debug messaging hooks
         */
        auto InitializeVulkan() -> vkb::Instance
        {
            vkb::InstanceBuilder builder;
            auto inst_ret = builder.set_app_name("SJ Game")
                                .request_validation_layers(g_IsDebugBuild)
#ifndef SJ_GOLD
                                .set_debug_callback(VulkanDebugLogCallback)
#endif
                                .require_api_version(1, 3, 0)
                                .build();

            vkb::Instance vkb_inst = inst_ret.value();
            m_vkInstance = vkb_inst.instance;
            m_vkDebugMessenger = vkb_inst.debug_messenger;

            SJ_ENGINE_LOG_INFO("Vulkan Instance Initialized");
            return vkb_inst;
        }

        VkCommandBuffer BeginSingleTimeCommands()
        {
            VkCommandBufferAllocateInfo allocInfo {};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = m_graphicsCommandPool;
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer {};
            vkAllocateCommandBuffers(m_renderDevice.GetLogicalDevice(), &allocInfo, &commandBuffer);

            VkCommandBufferBeginInfo beginInfo {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(commandBuffer, &beginInfo);

            return commandBuffer;
        }

        void EndSingleTimeCommands(VkCommandBuffer& commandBuffer)
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

        void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
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

            VkPipelineStageFlags sourceStage = 0;
            VkPipelineStageFlags destinationStage = 0;
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
                sourceStage = VK_IMAGE_LAYOUT_UNDEFINED;
                destinationStage = VK_IMAGE_LAYOUT_UNDEFINED;
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

        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
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

            region.imageOffset = {.x = 0, .y = 0, .z = 0};
            region.imageExtent = {.width = width, .height = height, .depth = 1};

            // "Assuming here that the image has already been transitioned to the layout that is
            // optimal for copying pixels to"
            vkCmdCopyBufferToImage(commandBuffer,
                                   buffer,
                                   image,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   1,
                                   &region);

            EndSingleTimeCommands(commandBuffer);
        }

        void CreateRenderTarget(Viewport windowSize)
        {
            VmaAllocationCreateInfo renderImageAllocInfo = {};
            renderImageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            renderImageAllocInfo.requiredFlags =
                VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            VkExtent3D drawImageExtent = {windowSize.Width, windowSize.Height, 1};

            VkImageUsageFlags drawImageUsages {};
            drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
            drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            m_drawImage = sj::vk::CreateImage(m_renderDevice.GetLogicalDevice(),
                                              m_renderDevice.GetAllocator(),
                                              renderImageAllocInfo,
                                              drawImageExtent,
                                              VK_FORMAT_R16G16B16A16_SFLOAT,
                                              drawImageUsages,
                                              VK_IMAGE_TILING_OPTIMAL);

            VkImageViewCreateInfo renderImageViewInfo =
                sj::vk::MakeImageViewCreateInfo(m_drawImage.imageFormat,
                                                m_drawImage.image,
                                                VK_IMAGE_ASPECT_COLOR_BIT);

            VkResult res = vkCreateImageView(m_renderDevice.GetLogicalDevice(),
                                             &renderImageViewInfo,
                                             g_vkAllocationFns,
                                             &m_drawImage.imageView);
        }

        void DestroyRenderTarget()
        {
            vkDestroyImageView(m_renderDevice.GetLogicalDevice(), m_drawImage.imageView, nullptr);
            vmaDestroyImage(m_renderDevice.GetAllocator(),
                            m_drawImage.image,
                            m_drawImage.allocation);
        }

        void CreateCommandPools()
        {
            VkCommandPoolCreateInfo poolInfo {};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolInfo.queueFamilyIndex = m_renderDevice.GetGraphicsQueueIndex();

            VkResult res = vkCreateCommandPool(m_renderDevice.GetLogicalDevice(),
                                               &poolInfo,
                                               sj::g_vkAllocationFns,
                                               &m_graphicsCommandPool);

            SJ_ASSERT(res == VK_SUCCESS, "Failed to create graphics command pool");

            res = vkCreateCommandPool(m_renderDevice.GetLogicalDevice(),
                                      &poolInfo,
                                      nullptr,
                                      &m_immediateCommandPool);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to create immediate command pool");

            VkCommandBufferAllocateInfo immCommandInfo =
                sj::vk::MakeCommandBufferAllocateInfo(m_immediateCommandPool, 1);
            res = vkAllocateCommandBuffers(m_renderDevice.GetLogicalDevice(),
                                           &immCommandInfo,
                                           &m_immediateCommandBuffer);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to allocate immediate command buffer");
        }

        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
        {
            VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

            VkBufferCopy copyRegion {};
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

            EndSingleTimeCommands(commandBuffer);
        }

        void LoadDummyModel()
        {
            std::ifstream modelFile;
            modelFile.open("Data/Engine/viking_room.sj_mesh", std::ios::in | std::ios::binary);
            SJ_ASSERT(modelFile.is_open(), "Failed to load model file!");

            Model header;
            modelFile.read(reinterpret_cast<char*>(&header), sizeof(header));

            // Read verts into GPU memory
            {
                VkDeviceSize bufferSizeBytes = sizeof(Vertex) * header.numVerts;
                // Stage vertex data in host visible buffer

                sj::vk::BufferResource stagingBuffer =
                    sj::vk::CreateBuffer(m_renderDevice.GetAllocator(),
                                         bufferSizeBytes,
                                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                         VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                             VMA_ALLOCATION_CREATE_MAPPED_BIT);

                // Copy data from file to GPU
                modelFile.read(reinterpret_cast<char*>(stagingBuffer.info.pMappedData),
                               bufferSizeBytes);

                m_dummyVertexBuffer = sj::vk::CreateBuffer(m_renderDevice.GetAllocator(),
                                                           bufferSizeBytes,
                                                           VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

                CopyBuffer(stagingBuffer.buffer, m_dummyVertexBuffer.buffer, bufferSizeBytes);

                vmaDestroyBuffer(m_renderDevice.GetAllocator(),
                                 stagingBuffer.buffer,
                                 stagingBuffer.allocation);
            }

            // Read Indices into GPU memory
            {
                m_dummyIndexBufferIndexCount = header.numIndices;

                VkDeviceSize bufferSizeBytes = sizeof(uint16_t) * header.numIndices;

                sj::vk::BufferResource stagingBuffer =
                    sj::vk::CreateBuffer(m_renderDevice.GetAllocator(),
                                         bufferSizeBytes,
                                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                         VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                             VMA_ALLOCATION_CREATE_MAPPED_BIT);

                // Copy data from file to GPU
                modelFile.read(reinterpret_cast<char*>(stagingBuffer.info.pMappedData),
                               bufferSizeBytes);

                m_dummyIndexBuffer = sj::vk::CreateBuffer(m_renderDevice.GetAllocator(),
                                                          bufferSizeBytes,
                                                          VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                              VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                CopyBuffer(stagingBuffer.buffer, m_dummyIndexBuffer.buffer, bufferSizeBytes);

                vmaDestroyBuffer(m_renderDevice.GetAllocator(),
                                 stagingBuffer.buffer,
                                 stagingBuffer.allocation);
            }

            modelFile.close();
        }

        void CreateDummyTextureImage()
        {
            std::ifstream textureFile;
            textureFile.open("Data/Engine/viking_room.sj_tex", std::ios::in | std::ios::binary);
            TextureHeader header;
            textureFile.read(reinterpret_cast<char*>(&header), sizeof(header));

            VkDeviceSize imageSizeBytes = header.width * header.height * 4;
            sj::vk::BufferResource stagingBuffer =
                sj::vk::CreateBuffer(m_renderDevice.GetAllocator(),
                                     imageSizeBytes,
                                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                     VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                         VMA_ALLOCATION_CREATE_MAPPED_BIT);

            // Read texture data straight into GPU memory
            textureFile.read(reinterpret_cast<char*>(stagingBuffer.info.pMappedData),
                             imageSizeBytes);
            textureFile.close();

            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            allocCreateInfo.requiredFlags =
                VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            VkExtent3D imageExtent = {static_cast<uint32_t>(header.width),
                                      static_cast<uint32_t>(header.height),
                                      1};

            m_dummyTextureImage =
                sj::vk::CreateImage(m_renderDevice.GetLogicalDevice(),
                                    m_renderDevice.GetAllocator(),
                                    allocCreateInfo,
                                    imageExtent,
                                    VK_FORMAT_R8G8B8A8_SRGB,
                                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                    VK_IMAGE_TILING_OPTIMAL);

            TransitionImageLayout(m_dummyTextureImage.image,
                                  VK_IMAGE_LAYOUT_UNDEFINED,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            CopyBufferToImage(stagingBuffer.buffer,
                              m_dummyTextureImage.image,
                              header.width,
                              header.height);

            TransitionImageLayout(m_dummyTextureImage.image,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            vmaDestroyBuffer(m_renderDevice.GetAllocator(),
                             stagingBuffer.buffer,
                             stagingBuffer.allocation);

            return;
        }

        void CreateDummyTextureSampler()
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

        void CreateGlobalDescriptorSetlayout()
        {
            scratchpad_scope scope = ThreadContext::GetScratchpad();

            sj::vk::DescriptorLayoutBuilder builder(&scope.get_allocator());
            builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
            builder.AddBinding(1,
                               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                               VK_SHADER_STAGE_FRAGMENT_BIT);

            m_globalUBODescriptorSetLayout =
                builder.Build(m_renderDevice.GetLogicalDevice(), nullptr);
        }

        void CreateGlobalUniformBuffers()
        {
            VkDeviceSize bufferSize = sizeof(GlobalUniformBufferObject);

            for(size_t i = 0; i < kMaxFramesInFlight; i++)
            {
                // This might not actaully end up being host visible:
                // https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/usage_patterns.html
                m_frameData.globalUniformBuffers[i] = sj::vk::CreateBuffer(
                    m_renderDevice.GetAllocator(),
                    bufferSize,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                        VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
                        VMA_ALLOCATION_CREATE_MAPPED_BIT);
            }
        }

        void CreateGlobalUBODescriptorPool()
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

            m_imguiDescriptorAllocator.InitPool(m_renderDevice.GetLogicalDevice(),
                                                1,
                                                pool_sizes,
                                                VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
        }
#endif // !SJ_GOLD

        void CreateGlobalUBODescriptorSets()
        {
            std::array<VkDescriptorSetLayout, kMaxFramesInFlight> layouts {};
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
                bufferInfo.buffer = m_frameData.globalUniformBuffers[i].buffer;
                bufferInfo.offset = 0;
                bufferInfo.range = sizeof(GlobalUniformBufferObject);

                VkDescriptorImageInfo imageInfo {};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = m_dummyTextureImage.imageView;
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

        template <class Fn>
            requires std::invocable<Fn, VkCommandBuffer>
        void ImmediateSubmit(Fn&& function)
        {
            VkResult res = vkResetFences(m_renderDevice.GetLogicalDevice(), 1, &m_immediateFence);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to reset immediate render fence!");

            res = vkResetCommandBuffer(m_immediateCommandBuffer, 0);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to reset immediate mode command buffer!");

            VkCommandBuffer cmd = m_immediateCommandBuffer;

            VkCommandBufferBeginInfo beginInfo =
                sj::vk::MakeCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

            res = vkBeginCommandBuffer(cmd, &beginInfo);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to start immediate mode command buffer!");

            std::forward<Fn>(function)(cmd);

            res = vkEndCommandBuffer(cmd);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to finalize immediate mode command buffer!");

            VkCommandBufferSubmitInfo submitCommandInfo = sj::vk::MakeCommandBufferSubmitInfo(cmd);

            VkSubmitInfo2 submitInfo = sj::vk::MakeSubmitInfo(&submitCommandInfo, nullptr, nullptr);

            res =
                vkQueueSubmit2(m_renderDevice.GetGraphicsQueue(), 1, &submitInfo, m_immediateFence);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to submit immediate command buffer!");

            res = vkWaitForFences(m_renderDevice.GetLogicalDevice(),
                                  1,
                                  &m_immediateFence,
                                  true,
                                  9999999999);
            SJ_ASSERT(res == VK_SUCCESS, "Immediate mode wait for fence timed out!");
        }

#ifndef SJ_GOLD
        void
        DrawImGui(VkCommandBuffer cmd, VkImageView targetImageView, VkExtent2D targetImageExtent)
        {
            VkRenderingAttachmentInfo colorAttachment =
                sj::vk::MakeAttachmentInfo(targetImageView,
                                           nullptr,
                                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            VkRenderingInfo renderInfo =
                sj::vk::MakeRenderingInfo(targetImageExtent, &colorAttachment, nullptr);

            vkCmdBeginRendering(cmd, &renderInfo);
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
            vkCmdEndRendering(cmd);
        }
#endif

        void DrawBackground(VkCommandBuffer cmd)
        {
            // Blue flashy clear color
            VkClearColorValue clearValue;
            float flash = std::abs(std::sin((float)m_frameCount / 240.f));
            clearValue = {{0.0f, 0.0f, flash, 1.0f}};

            VkImageSubresourceRange clearRange =
                sj::vk::MakeImageSubResourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

            vkCmdClearColorImage(cmd,
                                 m_drawImage.image,
                                 VK_IMAGE_LAYOUT_GENERAL,
                                 &clearValue,
                                 1,
                                 &clearRange);
        }

        void DrawGeometry(VkCommandBuffer cmd)
        {
            VkRenderingAttachmentInfo colorAttachment =
                sj::vk::MakeAttachmentInfo(m_drawImage.imageView,
                                           nullptr,
                                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            VkRenderingInfo renderInfo = sj::vk::MakeRenderingInfo(
                {m_drawImage.imageExtent.width, m_drawImage.imageExtent.height},
                &colorAttachment,
                nullptr);

            vkCmdBindPipeline(cmd,
                              VK_PIPELINE_BIND_POINT_GRAPHICS,
                              m_defaultPipeline.GetPipeline());

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

            vkCmdDrawIndexed(cmd, static_cast<uint32_t>(m_dummyIndexBufferIndexCount), 1, 0, 0, 0);

            vkCmdEndRendering(cmd);
        }

        void RecordDrawCommands(VkCommandBuffer buffer, uint32_t frameIdx, uint32_t imageIdx)
        {
            VkCommandBufferBeginInfo beginInfo =
                sj::vk::MakeCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
            VkResult res = vkBeginCommandBuffer(buffer, &beginInfo);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to start command buffer!");

            // Make swapchain image writable
            sj::vk::TransitionImage(buffer,
                                    m_drawImage.image,
                                    VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_GENERAL);

            DrawBackground(buffer);

            sj::vk::TransitionImage(buffer,
                                    m_drawImage.image,
                                    VK_IMAGE_LAYOUT_GENERAL,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

            // DrawGeometry(buffer);

            // transition the draw image and the swapchain image into their correct transfer layouts
            sj::vk::TransitionImage(buffer,
                                    m_drawImage.image,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

            VkImage swapChainImage = m_swapChain.GetImage(imageIdx);
            sj::vk::TransitionImage(buffer,
                                    swapChainImage,
                                    VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            // Copy working image into swapchain
            sj::vk::CopyImageToImage(buffer,
                                     m_drawImage.image,
                                     swapChainImage,
                                     m_drawExtent,
                                     m_swapChain.GetExtent());

            // Put swapchain image back into color attachment mode
            sj::vk::TransitionImage(buffer,
                                    swapChainImage,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

#ifndef SJ_GOLD
            DrawImGui(buffer, m_swapChain.GetImageView(imageIdx), m_swapChain.GetExtent());
#endif

            // Make swapchain image ready for presentation
            sj::vk::TransitionImage(buffer,
                                    swapChainImage,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

            res = vkEndCommandBuffer(buffer);
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
            ubo.projection = PerspectiveProjection(ToRadians(45.0f), aspectRatio, 0.1f, 100.0f);
            memcpy(bufferMem, &ubo, sizeof(ubo));
        }

        /** The Vulkan instance is the engine's connection to the vulkan library */
        VkInstance m_vkInstance {};

        sj::vk::AllocatedImage m_drawImage = {};
        VkExtent2D m_drawExtent = {};

        /** Handle to manage Vulkan's debug callbacks */
        VkDebugUtilsMessengerEXT m_vkDebugMessenger {};

        /** Handle to the surface vulkan renders to */
        VkSurfaceKHR m_renderingSurface {};

        /** Used to back API operations */
        sj::vk::RenderDevice m_renderDevice;

        /** Used for image presentation */
        sj::vk::SwapChain m_swapChain;

        /** Pipeline used to describe rendering process */
        sj::vk::Pipeline m_defaultPipeline {};

        VkCommandPool m_graphicsCommandPool {};

        sj::vk::BufferResource m_dummyVertexBuffer {};

        uint64_t m_dummyIndexBufferIndexCount {};
        sj::vk::BufferResource m_dummyIndexBuffer {};

        sj::vk::AllocatedImage m_dummyTextureImage {};
        VkSampler m_dummyTextureSampler {};

        VkDescriptorSetLayout m_globalUBODescriptorSetLayout {};
        VkDescriptorPool m_globalUBODescriptorPool {};

        sj::vk::DescriptorAllocator m_imguiDescriptorAllocator;

        VkFence m_immediateFence;
        VkCommandBuffer m_immediateCommandBuffer;
        VkCommandPool m_immediateCommandPool;

        /**
         * Data representing a render frames in flight
         */
        struct RenderFrameData
        {
            std::array<VkCommandBuffer, kMaxFramesInFlight> commandBuffers;

            std::array<VkSemaphore, kMaxFramesInFlight> imageAvailableSemaphores;
            std::array<VkFence, kMaxFramesInFlight> inFlightFences;

            std::array<sj::vk::BufferResource, kMaxFramesInFlight> globalUniformBuffers;
            std::array<VkDescriptorSet, kMaxFramesInFlight> globalUBODescriptorSets;

            void Init(VkDevice device, VkCommandPool commandPool)
            {
                // Create Command Buffer
                {
                    VkCommandBufferAllocateInfo allocInfo {};
                    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                    allocInfo.commandPool = commandPool;
                    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

                    VkResult res =
                        vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());

                    SJ_ASSERT(res == VK_SUCCESS, "Failed to create graphics command buffer");
                }

                // Create Sync Primitives
                {
                    VkSemaphoreCreateInfo semaphoreInfo {};
                    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

                    VkFenceCreateInfo fenceInfo {};
                    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

                    for(uint32_t i = 0; i < kMaxFramesInFlight; i++)
                    {
                        VkResult res = vkCreateSemaphore(device,
                                                         &semaphoreInfo,
                                                         sj::g_vkAllocationFns,
                                                         &imageAvailableSemaphores[i]);
                        SJ_ASSERT(res == VK_SUCCESS, "Failed to create syncronization primitive");

                        res = vkCreateFence(device,
                                            &fenceInfo,
                                            sj::g_vkAllocationFns,
                                            &inFlightFences[i]);
                        SJ_ASSERT(res == VK_SUCCESS, "Failed to create syncronization primitive");
                    }
                }
            }

            void DeInit(sj::vk::RenderDevice& device)
            {
                // NOTE: Command buffers are freed for us when we free the command pool.
                //       We only need to clean up sync primitives

                VkDevice logicalDevice = device.GetLogicalDevice();
                VmaAllocator allocator = device.GetAllocator();
                for(uint32_t i = 0; i < kMaxFramesInFlight; i++)
                {
                    vkDestroySemaphore(logicalDevice,
                                       imageAvailableSemaphores[i],
                                       sj::g_vkAllocationFns);
                    vkDestroyFence(logicalDevice, inFlightFences[i], sj::g_vkAllocationFns);

                    vmaDestroyBuffer(allocator,
                                     globalUniformBuffers[i].buffer,
                                     globalUniformBuffers[i].allocation);
                }
            }
        } m_frameData {};

        uint32_t m_frameCount = 0;
    };
} // namespace sj