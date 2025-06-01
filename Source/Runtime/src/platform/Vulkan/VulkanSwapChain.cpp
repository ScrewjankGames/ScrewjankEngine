// Parent Include
#include <ScrewjankEngine/platform/Vulkan/VulkanSwapChain.hpp>

// Screwjank Headers
#include <ScrewjankEngine/platform/Vulkan/VulkanHelpers.hpp>
#include <ScrewjankEngine/framework/Window.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanRenderDevice.hpp>
#include <ScrewjankEngine/rendering/Renderer.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanHelpers.hpp>

#include <ScrewjankShared/utils/Assert.hpp>

namespace sj
{
    VulkanSwapChain::VulkanSwapChain() : 
        m_images(Renderer::WorkBuffer()),
        m_imageViews(Renderer::WorkBuffer()),
        m_swapChainBuffers(Renderer::WorkBuffer())
    {

    }

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

    void VulkanSwapChain::Init(VkPhysicalDevice physicalDevice,
                               VkDevice logicalDevice,
                               VkSurfaceKHR renderingSurface)
    {
        SJ_ASSERT(!m_isInitialized, "Double initialization detected!");

        m_targetWindow = Window::GetInstance();

        SwapChainParams params = QuerySwapChainParams(physicalDevice, renderingSurface);
        
        VkSurfaceFormatKHR selected_format = ChoseSurfaceFormat(params.Formats);
        VkPresentModeKHR present_mode = ChosePresentMode(params.PresentModes);
        VkExtent2D extent = QuerySwapExtent(params.Capabilities);

        SJ_ASSERT(selected_format.format != VK_FORMAT_UNDEFINED,
                  "Failed to select swap chain surface format");

        uint32_t image_count = params.Capabilities.minImageCount + 1;
        if(params.Capabilities.maxImageCount > 0 && image_count > params.Capabilities.maxImageCount)
        {
            image_count = params.Capabilities.maxImageCount;
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

        DeviceQueueFamilyIndices indices =
            VulkanRenderDevice::GetDeviceQueueFamilyIndices(physicalDevice, renderingSurface);

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
        VkResult swap_chain_create_success =
            vkCreateSwapchainKHR(
                logicalDevice, 
                &create_info, 
                sj::g_vkAllocationFns, 
                &m_swapChain
            );

        SJ_ASSERT(VkResult::VK_SUCCESS == swap_chain_create_success, "Failed to create swapchain");

        // Extract swap chain image handles
        uint32_t real_image_count = 0;
        vkGetSwapchainImagesKHR(logicalDevice, m_swapChain, &real_image_count, nullptr);

        m_images.resize(real_image_count);

        vkGetSwapchainImagesKHR(logicalDevice,
                                m_swapChain,
                                &real_image_count,
                                m_images.data());

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

    void VulkanSwapChain::InitFrameBuffers(VkDevice device, VkRenderPass pass)
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

    void VulkanSwapChain::DeInit(VkDevice logicalDevice)
    {
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

    void VulkanSwapChain::Recreate(VkPhysicalDevice physicalDevice,
                                   VkDevice device,
                                   VkSurfaceKHR renderingSurface,
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
        Init(physicalDevice, device, renderingSurface);
        InitFrameBuffers(device, pass);
    }

    VkExtent2D VulkanSwapChain::GetExtent() const
    {
        return m_imageExtent;
    }

    VkFormat VulkanSwapChain::GetImageFormat() const
    {
        return m_chainImageFormat;
    }

    VkSwapchainKHR VulkanSwapChain::GetSwapChain() const
    {
        return m_swapChain;
    }

    VkExtent2D VulkanSwapChain::QuerySwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }

        Viewport frame_buffer_size = m_targetWindow->GetViewportSize();
        VkExtent2D extent {frame_buffer_size.Width, frame_buffer_size.Height};

        // Clamp values to max supported by implementation
        extent.width = std::max(capabilities.minImageExtent.width,
                                std::min(capabilities.maxImageExtent.width, extent.width));

        extent.height = std::max(capabilities.minImageExtent.height,
                                 std::min(capabilities.maxImageExtent.height, extent.height));

        return extent;
    }

    void VulkanSwapChain::CreateDepthResources(VkDevice logicalDevice,
                                               VkPhysicalDevice physicalDevice)
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

        m_depthImageView = CreateImageView(logicalDevice, m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    VulkanSwapChain::SwapChainParams
    VulkanSwapChain::QuerySwapChainParams(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
    {
        VkSurfaceCapabilitiesKHR capabilities;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device,
                                                  surface, &capabilities);

        uint32_t format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
                                             &format_count,
                                             nullptr);
        SJ_ASSERT(format_count != 0, "No surface formats found");

        uint32_t present_mode_count = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
                                                  surface,
                                                  &present_mode_count,
                                                  nullptr);
        SJ_ASSERT(present_mode_count != 0, "No present modes found");

        sj::memory_resource* currMemSpace = Renderer::WorkBuffer();
        SwapChainParams params
        {
            .Capabilities=capabilities,
            .Formats=dynamic_array<VkSurfaceFormatKHR>(format_count, currMemSpace),
            .PresentModes=dynamic_array<VkPresentModeKHR>(present_mode_count, currMemSpace),
        };

        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,
                                             surface,
                                             &format_count,
                                             params.Formats.data());

        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
                                                  surface,
                                                  &present_mode_count,
                                                  params.PresentModes.data());

        return params;
    }

    std::span<VkImageView> VulkanSwapChain::GetImageViews() const
    {
        return std::span<VkImageView>(m_imageViews.data(), m_imageViews.size());
    }

    std::span<VkFramebuffer> VulkanSwapChain::GetFrameBuffers() const
    {
        return std::span<VkFramebuffer>(m_swapChainBuffers.data(), m_imageViews.size());
    }
} // namespace sj