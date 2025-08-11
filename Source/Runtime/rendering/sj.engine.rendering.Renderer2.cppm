module;

#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/PlatformDetection.hpp>
#include <ScrewjankEngine/framework/Window.hpp>

#include <vulkan/vulkan_core.h>
#include <VkBootstrap.h>

#include <cstdint>

export module sj.engine.rendering.Renderer2;
import sj.engine.rendering.vk.Helpers;
import sj.engine.rendering.vk.Utils;
import sj.engine.framework.Engine;
import sj.std.containers.array;

export namespace sj
{
    class Renderer2
    {
    public:
        Renderer2() = default;

        void Init()
        {
            InitVulkan();
            InitCommands();
        }

        void DeInit()
        {
            if(m_instance == VK_NULL_HANDLE)
                return;

            // make sure the gpu has stopped doing its things
            vkDeviceWaitIdle(m_logicalDevice);

            for(FrameData& frame : m_frameData)
                vkDestroyCommandPool(m_logicalDevice, frame.commandPool, nullptr);

            DestroySwapchain();

            vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
            vkDestroyDevice(m_logicalDevice, nullptr);

            vkb::destroy_debug_utils_messenger(m_instance, m_vkDebugMessenger);
            vkDestroyInstance(m_instance, nullptr);
        }

        void Render()
        {
            FrameData& renderFrame = GetCurrentFrameData();
            [[maybe_unused]] VkResult res = VK_SUCCESS;

            res = vkWaitForFences(m_logicalDevice, 1, &renderFrame.renderFence, true, 1000000000);
            SJ_ASSERT(res == VK_SUCCESS,
                      "Renderer waited more than 1 second for draw to complete. Is it dead?");

            res = vkResetFences(m_logicalDevice, 1, &renderFrame.renderFence);
            SJ_ASSERT(res == VK_SUCCESS, "Failed to reset render fence");

            // request image from the swapchain
            [[indeterminate]] uint32_t swapchainImageIndex;
            res = vkAcquireNextImageKHR(m_logicalDevice,
                                           m_swapchain,
                                           1000000000,
                                           renderFrame.swapchainSemaphore,
                                           nullptr,
                                           &swapchainImageIndex);
        }

    private:
        struct FrameData
        {
            VkCommandPool commandPool;
            VkCommandBuffer commandBuffer;

            VkSemaphore swapchainSemaphore;
            VkSemaphore renderSemaphore;
            VkFence renderFence;
        };

        void InitVulkan()
        {
            vkb::InstanceBuilder builder;
            auto inst_ret = builder.set_app_name(Engine::GetGameName().data())
                                .request_validation_layers(g_IsDebugBuild)
                                .set_debug_callback(VulkanDebugLogCallback)
                                .require_api_version(1, 3, 0)
                                .build();

            vkb::Instance vkb_inst = inst_ret.value();
            m_instance = vkb_inst.instance;
            m_vkDebugMessenger = vkb_inst.debug_messenger;

            m_surface = Window::GetInstance()->CreateWindowSurface(m_instance);

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

            // use vkbootstrap to select a gpu.
            vkb::PhysicalDeviceSelector selector {vkb_inst};
            vkb::PhysicalDevice physicalDevice = selector.set_minimum_version(1, 3)
                                                     .set_required_features_13(features1_3)
                                                     .set_required_features_12(features1_2)
                                                     .set_surface(m_surface)
                                                     .select()
                                                     .value();

            // create the final vulkan device
            vkb::DeviceBuilder deviceBuilder {physicalDevice};
            vkb::Device vkbDevice = deviceBuilder.build().value();

            // Get the VkDevice handle used in the rest of a vulkan application
            m_logicalDevice = vkbDevice.device;
            m_physicalDevice = physicalDevice.physical_device;

            // Grab graphics queue info
            m_graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
            m_graphicsQueueFamilyIndex =
                vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
        }

        void InitCommands()
        {
            // create a command pool for commands submitted to the graphics queue.
            // we also want the pool to allow for resetting of individual command buffers
            VkCommandPoolCreateInfo commandPoolInfo =
                sj::vk::MakeCommandPoolCreateInfo(m_graphicsQueueFamilyIndex,
                                                  VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

            for(FrameData& frame : m_frameData)
            {
                [[maybe_unused]] VkResult res = VK_SUCCESS;
                res = vkCreateCommandPool(m_logicalDevice,
                                          &commandPoolInfo,
                                          nullptr,
                                          &frame.commandPool);
                SJ_ASSERT(res == VK_SUCCESS, "Failed to initialize command pool");

                // allocate the default command buffer that we will use for rendering
                VkCommandBufferAllocateInfo cmdAllocInfo =
                    sj::vk::MakeCommandBufferAllocateInfo(frame.commandPool, 1);

                res =
                    vkAllocateCommandBuffers(m_logicalDevice, &cmdAllocInfo, &frame.commandBuffer);
            }
        }

        void CreateSwapchain(uint32_t width, uint32_t height)
        {
            vkb::SwapchainBuilder swapchainBuilder {m_physicalDevice, m_logicalDevice, m_surface};
            m_swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

            vkb::Swapchain vkbSwapchain =
                swapchainBuilder
                    //.use_default_format_selection()
                    .set_desired_format(
                        VkSurfaceFormatKHR {.format = m_swapchainImageFormat,
                                            .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
                    // use vsync present mode
                    .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                    .set_desired_extent(width, height)
                    .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                    .build()
                    .value();

            m_swapchainExtent = vkbSwapchain.extent;

            // store swapchain and its related images
            m_swapchain = vkbSwapchain.swapchain;

            const auto& tmpImages = vkbSwapchain.get_images().value();
            m_swapchainImages.resize(tmpImages.size());
            std::ranges::copy(tmpImages, m_swapchainImages.begin());

            const auto& tmpImageViews = vkbSwapchain.get_image_views().value();
            m_swapchainImageViews.resize(tmpImageViews.size());
            std::ranges::copy(tmpImageViews, m_swapchainImageViews.begin());
        }

        void DestroySwapchain()
        {
            vkDestroySwapchainKHR(m_logicalDevice, m_swapchain, nullptr);

            for(VkImageView view : m_swapchainImageViews)
            {
                vkDestroyImageView(m_logicalDevice, view, nullptr);
            }
        }

        void InitSyncStructures()
        {
            VkFenceCreateInfo fenceCreateInfo =
                sj::vk::MakeFenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
            VkSemaphoreCreateInfo semaphoreCreateInfo = sj::vk::MakeSemaphoreCreateInfo();

            for(FrameData& frame : m_frameData)
            {
                [[maybe_unused]] VkResult res = VK_SUCCESS;

                res = vkCreateFence(m_logicalDevice, &fenceCreateInfo, nullptr, &frame.renderFence);
                SJ_ASSERT(res == VK_SUCCESS, "Failed to create rendering fence");

                res = vkCreateSemaphore(m_logicalDevice,
                                        &semaphoreCreateInfo,
                                        nullptr,
                                        &frame.swapchainSemaphore);
                SJ_ASSERT(res == VK_SUCCESS, "Failed to create rendering semaphore");

                res = vkCreateSemaphore(m_logicalDevice,
                                        &semaphoreCreateInfo,
                                        nullptr,
                                        &frame.renderSemaphore);

                SJ_ASSERT(res == VK_SUCCESS, "Failed to create rendering semaphore");
            }
        }

        FrameData& GetCurrentFrameData()
        {
            return m_frameData[m_frameNumber % kFrameOverlap];
        }

        VkInstance m_instance = VK_NULL_HANDLE;

        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice m_logicalDevice = VK_NULL_HANDLE;

        VkSurfaceKHR m_surface = VK_NULL_HANDLE;

        VkDebugUtilsMessengerEXT m_vkDebugMessenger = VK_NULL_HANDLE;

        VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
        VkFormat m_swapchainImageFormat = VK_FORMAT_UNDEFINED;

        sj::dynamic_array<VkImage> m_swapchainImages;
        sj::dynamic_array<VkImageView> m_swapchainImageViews;
        VkExtent2D m_swapchainExtent = {};

        uint64_t m_frameNumber = 0;
        static constexpr int kFrameOverlap = 2;
        std::array<FrameData, kFrameOverlap> m_frameData = {};

        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        uint32_t m_graphicsQueueFamilyIndex = -1;
    };
} // namespace sj