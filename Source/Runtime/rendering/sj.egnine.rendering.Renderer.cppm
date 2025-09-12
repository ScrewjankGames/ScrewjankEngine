module;

// Screwjank Headers
#include <ScrewjankEngine/framework/Window.hpp>

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
import sj.datadefs.assets.Mesh;
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
                                   m_drawImage.GetImageFormat(),
                                   "Data/Engine/Shaders/Default.vert.spv",
                                   "Data/Engine/Shaders/Default.frag.spv");

            CreateCommandPools();
            m_immediateCommandContext.Init(m_renderDevice.GetLogicalDevice(),
                                           m_renderDevice.GetGraphicsQueueIndex(),
                                           m_renderDevice.GetGraphicsQueue());

            m_dummyMeshBuffers.Init("Data/Engine/viking_room.sj_mesh",
                                    m_renderDevice.GetAllocator(),
                                    m_immediateCommandContext);

            CreateDummyTextureImage();
            CreateDummyTextureSampler();

            CreateGlobalUniformBuffers();

            CreateGlobalUBODescriptorPool();

#ifndef SJ_GOLD
            CreateImGuiDescriptorPool();
#endif // !SJ_GOLD

            CreateGlobalUBODescriptorSets();

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
            init_info.MinImageCount = sj::vk::kMaxFramesInFlight;
            init_info.ImageCount = sj::vk::kMaxFramesInFlight;
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

            m_immediateCommandContext.DeInit();

            m_frameData.DeInit(m_renderDevice);

            vkDestroyCommandPool(logicalDevice, m_graphicsCommandPool, sj::g_vkAllocationFns);

            vkDestroyImageView(m_renderDevice.GetLogicalDevice(),
                               m_drawImageView,
                               g_vkAllocationFns);
            m_drawImage.DeInit(m_renderDevice.GetAllocator());

            m_swapChain.DeInit(m_renderDevice);

            vkDestroySampler(logicalDevice, m_dummyTextureSampler, sj::g_vkAllocationFns);
            vkDestroyImageView(logicalDevice, m_dummyTextureImageView, sj::g_vkAllocationFns);
            m_dummyTextureImage.DeInit(m_renderDevice.GetAllocator());

            m_globalDescriptorAllocator.DeInit(logicalDevice);

#ifndef SJ_GOLD
            m_imguiDescriptorAllocator.DeInit(logicalDevice);
#endif

            vkDestroyDescriptorSetLayout(logicalDevice,
                                         m_globalUBODescriptorSetLayout,
                                         sj::g_vkAllocationFns);

            m_defaultPipeline.DeInit(logicalDevice);

            m_dummyMeshBuffers.DeInit(m_renderDevice.GetAllocator());

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

            const uint32_t frameIdx = m_frameCount % sj::vk::kMaxFramesInFlight;
            sj::vk::FrameGlobals currFrameData = m_frameData.GetFrameGlobals(frameIdx);

            vkWaitForFences(m_renderDevice.GetLogicalDevice(),
                            1,
                            &currFrameData.fence,
                            VK_TRUE,
                            std::numeric_limits<uint64_t>::max());

            VkSwapchainKHR swapChain = m_swapChain.GetSwapChain();
            uint32_t imageIndex = 0;
            VkResult res = vkAcquireNextImageKHR(m_renderDevice.GetLogicalDevice(),
                                                 swapChain,
                                                 std::numeric_limits<uint64_t>::max(),
                                                 currFrameData.imgAvailableSemaphore,
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
            vkResetFences(m_renderDevice.GetLogicalDevice(), 1, &currFrameData.fence);

            vkResetCommandBuffer(currFrameData.cmd, 0);

            UpdateUniformBuffer(currFrameData.globalUniformBuffer.GetMappedMemory(), cameraMatrix);

            VkImage swapChainImage = m_swapChain.GetImage(imageIndex);
            VkImageView swapChainImageView = m_swapChain.GetImageView(imageIndex);
            RecordDrawCommands(currFrameData, swapChainImage, swapChainImageView);

            VkCommandBufferSubmitInfo submitCommandInfo =
                sj::vk::MakeCommandBufferSubmitInfo(currFrameData.cmd);

            VkSemaphoreSubmitInfo waitInfo =
                sj::vk::MakeSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
                                                currFrameData.imgAvailableSemaphore);

            VkSemaphoreSubmitInfo signalInfo =
                sj::vk::MakeSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
                                                currRenderFinishedSemaphore);

            VkSubmitInfo2 submitInfo =
                sj::vk::MakeSubmitInfo(&submitCommandInfo, &signalInfo, &waitInfo);

            res = vkQueueSubmit2(m_renderDevice.GetGraphicsQueue(),
                                 1,
                                 &submitInfo,
                                 currFrameData.fence);
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
            
#ifndef SJ_GOLD
            m_vkDebugMessenger = vkb_inst.debug_messenger;
#endif
            SJ_ENGINE_LOG_INFO("Vulkan Instance Initialized");
            return vkb_inst;
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

            m_drawImage = sj::vk::ImageResource(m_renderDevice.GetAllocator(),
                                                renderImageAllocInfo,
                                                drawImageExtent,
                                                VK_FORMAT_R16G16B16A16_SFLOAT,
                                                drawImageUsages,
                                                VK_IMAGE_TILING_OPTIMAL);

            m_drawImageView = m_drawImage.MakeImageView(m_renderDevice.GetLogicalDevice(),
                                                        VK_IMAGE_ASPECT_COLOR_BIT);
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
        }

        void CreateDummyTextureImage()
        {
            std::ifstream textureFile;
            textureFile.open("Data/Engine/viking_room.sj_tex", std::ios::in | std::ios::binary);
            TextureHeader header;
            textureFile.read(reinterpret_cast<char*>(&header), sizeof(header));

            VkDeviceSize imageSizeBytes = header.width * header.height * 4;
            sj::vk::BufferResource stagingBuffer =
                sj::vk::MakeStagingBuffer(m_renderDevice.GetAllocator(), imageSizeBytes);

            // Read texture data straight into GPU memory
            textureFile.read(reinterpret_cast<char*>(stagingBuffer.GetMappedMemory()),
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
                sj::vk::ImageResource(m_renderDevice.GetAllocator(),
                                      allocCreateInfo,
                                      imageExtent,
                                      VK_FORMAT_R8G8B8A8_SRGB,
                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                      VK_IMAGE_TILING_OPTIMAL);

            VkImage textureImage = m_dummyTextureImage.GetImage();
            m_immediateCommandContext.ImmediateSubmit([textureImage](VkCommandBuffer cmd) {
                sj::vk::TransitionImage(cmd,
                                        textureImage,
                                        VK_IMAGE_LAYOUT_UNDEFINED,
                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            });

            m_immediateCommandContext.ImmediateSubmit([buf = stagingBuffer.GetBuffer(),
                                                       img = m_dummyTextureImage.GetImage(),
                                                       header](VkCommandBuffer cmd) {
                sj::vk::CopyBufferToImage(cmd, buf, img, header.width, header.height);
            });

            m_immediateCommandContext.ImmediateSubmit([textureImage](VkCommandBuffer cmd) {
                sj::vk::TransitionImage(cmd,
                                        textureImage,
                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            });

            stagingBuffer.DeInit(m_renderDevice.GetAllocator());

            m_dummyTextureImageView =
                m_dummyTextureImage.MakeImageView(m_renderDevice.GetLogicalDevice(),
                                                  VK_IMAGE_ASPECT_COLOR_BIT);

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

            for(size_t i = 0; i < sj::vk::kMaxFramesInFlight; i++)
            {
                // This might not actaully end up being host visible:
                // https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/usage_patterns.html
                m_frameData.globalUniformBuffers[i] = sj::vk::BufferResource(
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
            std::array poolSizes {
                VkDescriptorPoolSize {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                      .descriptorCount = sj::vk::kMaxFramesInFlight},
                VkDescriptorPoolSize {.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                      .descriptorCount = sj::vk::kMaxFramesInFlight}};

            m_globalDescriptorAllocator.InitPool(m_renderDevice.GetLogicalDevice(),
                                                 sj::vk::kMaxFramesInFlight,
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

            m_imguiDescriptorAllocator.InitPool(m_renderDevice.GetLogicalDevice(),
                                                1,
                                                pool_sizes,
                                                VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
        }
#endif // !SJ_GOLD

        void CreateGlobalUBODescriptorSets()
        {
            std::array<VkDescriptorSetLayout, sj::vk::kMaxFramesInFlight> layouts {};
            std::ranges::fill(layouts, m_globalUBODescriptorSetLayout);

            m_globalDescriptorAllocator.Allocate(m_renderDevice.GetLogicalDevice(),
                                                 layouts,
                                                 m_frameData.globalUBODescriptorSets);

            for(size_t i = 0; i < sj::vk::kMaxFramesInFlight; i++)
            {
                VkDescriptorBufferInfo bufferInfo {};
                bufferInfo.buffer = m_frameData.globalUniformBuffers[i].GetBuffer();
                bufferInfo.offset = 0;
                bufferInfo.range = sizeof(GlobalUniformBufferObject);

                VkDescriptorImageInfo imageInfo {};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = m_dummyTextureImageView;
                imageInfo.sampler = m_dummyTextureSampler;

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
                                          .descriptorType =
                                              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                          .pImageInfo = &imageInfo}

                };

                vkUpdateDescriptorSets(m_renderDevice.GetLogicalDevice(),
                                       static_cast<uint32_t>(descriptorWrites.size()),
                                       descriptorWrites.data(),
                                       0,
                                       nullptr);
            }
        }

#ifndef SJ_GOLD
        void DrawImGui(VkCommandBuffer cmd,
                       VkDescriptorSet globalDescriptorSet,
                       VkImageView targetImageView,
                       VkExtent2D targetImageExtent)
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
                                 m_drawImage.GetImage(),
                                 VK_IMAGE_LAYOUT_GENERAL,
                                 &clearValue,
                                 1,
                                 &clearRange);
        }

        void DrawGeometry(VkCommandBuffer cmd, VkDescriptorSet globalDescriptorSet)
        {
            VkRenderingAttachmentInfo colorAttachment =
                sj::vk::MakeAttachmentInfo(m_drawImageView,
                                           nullptr,
                                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

            VkRenderingInfo renderInfo = sj::vk::MakeRenderingInfo(
                {m_drawImage.GetExtent().width, m_drawImage.GetExtent().height},
                &colorAttachment,
                nullptr);

            vkCmdBeginRendering(cmd, &renderInfo);

            vkCmdBindPipeline(cmd,
                              VK_PIPELINE_BIND_POINT_GRAPHICS,
                              m_defaultPipeline.GetPipeline());

            std::array<VkDescriptorSet, 1> descriptorSets = {globalDescriptorSet};
            vkCmdBindDescriptorSets(cmd,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    m_defaultPipeline.GetLayout(),
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

        void RecordDrawCommands(sj::vk::FrameGlobals& frameData,
                                VkImage swapchainImage,
                                VkImageView swapchainImageView)
        {
            VkCommandBufferBeginInfo beginInfo =
                sj::vk::MakeCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
            VkResult res = vkBeginCommandBuffer(frameData.cmd, &beginInfo);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to start command buffer!");

            // Make swapchain image writable
            sj::vk::TransitionImage(frameData.cmd,
                                    m_drawImage.GetImage(),
                                    VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_GENERAL);

            DrawBackground(frameData.cmd);

            sj::vk::TransitionImage(frameData.cmd,
                                    m_drawImage.GetImage(),
                                    VK_IMAGE_LAYOUT_GENERAL,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

            DrawGeometry(frameData.cmd, frameData.globalDescriptorSet);

            // transition the draw image and the swapchain image into their correct transfer layouts
            sj::vk::TransitionImage(frameData.cmd,
                                    m_drawImage.GetImage(),
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

            sj::vk::TransitionImage(frameData.cmd,
                                    swapchainImage,
                                    VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            // Copy working image into swapchain
            sj::vk::CopyImageToImage(frameData.cmd,
                                     m_drawImage.GetImage(),
                                     swapchainImage,
                                     m_drawImage.GetExtent2D(),
                                     m_swapChain.GetExtent());

            // Put swapchain image back into color attachment mode
            sj::vk::TransitionImage(frameData.cmd,
                                    swapchainImage,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

#ifndef SJ_GOLD
            DrawImGui(frameData.cmd,
                      frameData.globalDescriptorSet,
                      swapchainImageView,
                      m_swapChain.GetExtent());
#endif

            // Make swapchain image ready for presentation
            sj::vk::TransitionImage(frameData.cmd,
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
            ubo.projection = PerspectiveProjection(ToRadians(45.0f), aspectRatio, 0.1f, 100.0f);
            memcpy(bufferMem, &ubo, sizeof(ubo));
        }

        /** The Vulkan instance is the engine's connection to the vulkan library */
        VkInstance m_vkInstance {};

        sj::vk::ImageResource m_drawImage;
        VkImageView m_drawImageView;



        /** Handle to the surface vulkan renders to */
        VkSurfaceKHR m_renderingSurface {};

        /** Used to back API operations */
        sj::vk::RenderDevice m_renderDevice;

        /** Used for image presentation */
        sj::vk::SwapChain m_swapChain;

        /** Pipeline used to describe rendering process */
        sj::vk::Pipeline m_defaultPipeline {};

        VkCommandPool m_graphicsCommandPool {};

        sj::vk::MeshBuffers m_dummyMeshBuffers;

        sj::vk::ImageResource m_dummyTextureImage {};
        VkImageView m_dummyTextureImageView;
        VkSampler m_dummyTextureSampler {};

        sj::vk::DescriptorAllocator m_globalDescriptorAllocator;
        VkDescriptorSetLayout m_globalUBODescriptorSetLayout {};

        sj::vk::ImmediateCommandContext m_immediateCommandContext;

        /**
         * Data representing a render frames in flight
         */
        sj::vk::RenderFrameData m_frameData {};

        uint32_t m_frameCount = 0;

#ifndef SJ_GOLD
        sj::vk::DescriptorAllocator m_imguiDescriptorAllocator;
        
        /** Handle to manage Vulkan's debug callbacks */
        VkDebugUtilsMessengerEXT m_vkDebugMessenger {};
#endif
    };
} // namespace sj