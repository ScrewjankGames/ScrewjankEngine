module;

// STD Headers
#include <span>

// Library Headers
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

// Screwjank Headers
#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankEngine/framework/Window.hpp>

export module sj.engine.rendering.vk.SwapChain;
import sj.engine.rendering.vk.Utils;
import sj.std.containers.array;
import sj.std.containers.vector;
import sj.std.memory;

namespace sj::vk
{
    VkSurfaceFormatKHR ChoseSurfaceFormat(const std::span<VkSurfaceFormatKHR> formats)
    {
        for(const VkSurfaceFormatKHR& format : formats)
        {
            if(format.format == VK_FORMAT_B8G8R8A8_SRGB &&
               format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return format;
            }
        }

        return formats[0];
    }

    VkPresentModeKHR ChosePresentMode(const std::span<VkPresentModeKHR>& present_modes)
    {
        for(const VkPresentModeKHR& mode : present_modes)
        {
            if(mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return mode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

} // namespace sj::vk

export namespace sj::vk
{
    class SwapChain
    {
    public:
        SwapChain(sj::memory_resource* cpu_resource)
            : m_cpuMemoryResource(cpu_resource), m_images(cpu_resource), m_imageViews(cpu_resource), m_swapChainBuffers(cpu_resource)
        {
        }

        ~SwapChain() = default;

        void
        Init(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR renderingSurface, Window* surfaceHost)
        {
            SJ_ASSERT(!m_isInitialized, "Double initialization detected!");

            SwapChainParams params = QuerySwapChainParams(physicalDevice, renderingSurface);

            VkSurfaceFormatKHR selected_format = ChoseSurfaceFormat(params.Formats);
            VkPresentModeKHR present_mode = ChosePresentMode(params.PresentModes);
            VkExtent2D extent = QuerySwapExtent(surfaceHost, params.Capabilities);

            SJ_ASSERT(selected_format.format != VK_FORMAT_UNDEFINED,
                      "Failed to select swap chain surface format");

            uint32_t image_count = params.Capabilities.minImageCount + 1;
            if(params.Capabilities.maxImageCount > 0 &&
               image_count > params.Capabilities.maxImageCount)
            {
                image_count = params.Capabilities.maxImageCount;
            }

            VkSemaphoreCreateInfo semaphoreInfo {};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            m_renderFinishedSemaphores.resize(image_count);
            for(VkSemaphore& semaphore : m_renderFinishedSemaphores)
            {
                VkResult res = vkCreateSemaphore(logicalDevice,
                                                 &semaphoreInfo,
                                                 sj::g_vkAllocationFns,
                                                 &semaphore);
                SJ_ASSERT(res == VK_SUCCESS, "Failed to create syncronization primitive");
            }

            // Fill out swap chain create info
            VkSwapchainCreateInfoKHR create_info {};
            create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            create_info.surface = renderingSurface;
            create_info.minImageCount = image_count;
            create_info.imageFormat = selected_format.format;
            create_info.imageColorSpace = selected_format.colorSpace;
            create_info.imageExtent = extent;
            create_info.imageArrayLayers = 1;
            create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            DeviceQueueFamilyIndices indices = GetDeviceQueueFamilyIndices(physicalDevice, renderingSurface);

            std::array queue_family_indices_array = {indices.graphicsFamilyIndex.value(),
                                                     indices.presentationFamilyIndex.value()};

            if(indices.graphicsFamilyIndex != indices.presentationFamilyIndex)
            {
                create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                create_info.queueFamilyIndexCount = 2;
                create_info.pQueueFamilyIndices = queue_family_indices_array.data();
            }
            else
            {
                create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                create_info.queueFamilyIndexCount = 0; // Optional
                create_info.pQueueFamilyIndices = nullptr; // Optional
            }

            create_info.preTransform = params.Capabilities.currentTransform;
            create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            create_info.presentMode = present_mode;
            create_info.clipped = VK_TRUE;
            create_info.oldSwapchain = VK_NULL_HANDLE;

            // Create Swap Chain
            VkResult swap_chain_create_success = vkCreateSwapchainKHR(logicalDevice,
                                                                      &create_info,
                                                                      sj::g_vkAllocationFns,
                                                                      &m_swapChain);

            SJ_ASSERT(VkResult::VK_SUCCESS == swap_chain_create_success,
                      "Failed to create swapchain");

            // Extract swap chain image handles
            uint32_t real_image_count = 0;
            vkGetSwapchainImagesKHR(logicalDevice, m_swapChain, &real_image_count, nullptr);

            m_images.resize(real_image_count);

            vkGetSwapchainImagesKHR(logicalDevice, m_swapChain, &real_image_count, m_images.data());

            m_chainImageFormat = selected_format.format;
            m_imageExtent = extent;

            // Create Image Views
            m_imageViews.resize(m_images.size());

            for(size_t i = 0; i < m_images.size(); i++)
            {
                m_imageViews[i] = CreateImageView(logicalDevice,
                                                  m_images[i],
                                                  m_chainImageFormat,
                                                  VK_IMAGE_ASPECT_COLOR_BIT);
            }

            CreateDepthResources(logicalDevice, physicalDevice);
        }

        /**
         * Called after init because info from the swap chain is need to create the render pass
         */
        void InitFrameBuffers(VkDevice device, VkRenderPass pass)
        {

            // Create Frame Buffers
            m_swapChainBuffers.resize(m_imageViews.size());

            int i = 0;
            for(VkImageView& view : m_imageViews)
            {
                std::array attachments = {view, m_depthImageView};

                VkFramebufferCreateInfo framebufferInfo {};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = pass;
                framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                framebufferInfo.pAttachments = attachments.data();
                framebufferInfo.width = GetExtent().width;
                framebufferInfo.height = GetExtent().height;
                framebufferInfo.layers = 1;

                VkResult res = vkCreateFramebuffer(device,
                                                   &framebufferInfo,
                                                   sj::g_vkAllocationFns,
                                                   &m_swapChainBuffers[i]);

                SJ_ASSERT(res == VK_SUCCESS, "Failed to construct frame buffers.");

                i++;
            }
        }

        void DeInit(VkDevice logicalDevice)
        {
            for(VkSemaphore& semaphore : m_renderFinishedSemaphores)
            {
                vkDestroySemaphore(logicalDevice, semaphore, sj::g_vkAllocationFns);
            }

            vkDestroyImageView(logicalDevice, m_depthImageView, sj::g_vkAllocationFns);
            vkDestroyImage(logicalDevice, m_depthImage, sj::g_vkAllocationFns);
            vkFreeMemory(logicalDevice, m_depthImageMemory, sj::g_vkAllocationFns);

            for(VkFramebuffer buffer : m_swapChainBuffers)
            {
                vkDestroyFramebuffer(logicalDevice, buffer, sj::g_vkAllocationFns);
            }

            for(VkImageView view : m_imageViews)
            {
                vkDestroyImageView(logicalDevice, view, sj::g_vkAllocationFns);
            }

            vkDestroySwapchainKHR(logicalDevice, m_swapChain, sj::g_vkAllocationFns);
        }

        void Recreate(VkPhysicalDevice physicalDevice,
                      VkDevice device,
                      VkSurfaceKHR renderingSurface,
                      Window* window,
                      VkRenderPass pass)
        {
            // Window is minimized. Can't recreate swap chain.
            if(Window::GetInstance()->GetViewportSize().Width == 0 ||
               Window::GetInstance()->GetViewportSize().Height == 0)
            {
                return;
            }

            vkDeviceWaitIdle(device);

            DeInit(device);
            Init(physicalDevice, device, renderingSurface, window);
            InitFrameBuffers(device, pass);
        }

        [[nodiscard]] VkExtent2D GetExtent() const
        {
            return m_imageExtent;
        }

        [[nodiscard]] VkFormat GetImageFormat() const
        {
            return m_chainImageFormat;
        }

        [[nodiscard]] VkSwapchainKHR GetSwapChain() const
        {
            return m_swapChain;
        }

        [[nodiscard]] std::span<VkImageView> GetImageViews() const
        {
            return std::span<VkImageView>(m_imageViews.data(), m_imageViews.size());
        }

        [[nodiscard]] std::span<VkFramebuffer> GetFrameBuffers() const
        {
            return std::span<VkFramebuffer>(m_swapChainBuffers.data(), m_imageViews.size());
        }

        [[nodiscard]] VkSemaphore GetImageRenderCompleteSemaphore(uint32_t imageIdx)
        {
            return m_renderFinishedSemaphores.at(imageIdx);
        }

    private:
        /**
         * Communicates with the window to query swap chain extents
         */
        VkExtent2D QuerySwapExtent(Window* targetWindow, const VkSurfaceCapabilitiesKHR& capabilities)
        {
            if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            {
                return capabilities.currentExtent;
            }

            Viewport frame_buffer_size = targetWindow->GetViewportSize();
            VkExtent2D extent {frame_buffer_size.Width, frame_buffer_size.Height};

            // Clamp values to max supported by implementation
            extent.width = std::max(capabilities.minImageExtent.width,
                                    std::min(capabilities.maxImageExtent.width, extent.width));

            extent.height = std::max(capabilities.minImageExtent.height,
                                     std::min(capabilities.maxImageExtent.height, extent.height));

            return extent;
        }

        void CreateDepthResources(VkDevice logicalDevice, VkPhysicalDevice physicalDevice)
        {
            VkFormat depthFormat = FindDepthFormat(physicalDevice);

            CreateImage(logicalDevice,
                        physicalDevice,
                        GetExtent().width,
                        GetExtent().height,
                        depthFormat,
                        VK_IMAGE_TILING_OPTIMAL,
                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        m_depthImage,
                        m_depthImageMemory);

            m_depthImageView = CreateImageView(logicalDevice,
                                               m_depthImage,
                                               depthFormat,
                                               VK_IMAGE_ASPECT_DEPTH_BIT);
        }

        bool m_isInitialized = false;

        sj::memory_resource* m_cpuMemoryResource = nullptr;

        /** Pointer to the swap chain that controls image presenation */
        VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;

        /** Semaphors to track when each image is presented  */
        dynamic_array<VkSemaphore> m_renderFinishedSemaphores;

        /** List of handles to images in the swap chain */
        dynamic_vector<VkImage> m_images;

        /** List of Image Views onto the images in the swap chain */
        dynamic_vector<VkImageView> m_imageViews;

        /** Frame buffers for images in the swap chain */
        dynamic_vector<VkFramebuffer> m_swapChainBuffers;

        /** Format for images in the swap chain */
        VkFormat m_chainImageFormat = VK_FORMAT_UNDEFINED;

        /** Size of the images in the swap chain (in pixels) */
        VkExtent2D m_imageExtent = {};

        /** Depth buffer resources */
        VkImage m_depthImage {};
        VkDeviceMemory m_depthImageMemory {};
        VkImageView m_depthImageView {};
    };

} // namespace sj::vk