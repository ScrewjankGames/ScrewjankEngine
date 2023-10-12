// Parent Include
#include <ScrewjankEngine/platform/Vulkan/VulkanSwapChain.hpp>

// STD Headers

// Library Headers

// Screwjank Headers
#include <ScrewjankEngine/core/Window.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanRendererAPI.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanRenderDevice.hpp>
#include <ScrewjankEngine/rendering/Renderer.hpp>

namespace sj
{
    VulkanSwapChain::VulkanSwapChain() : 
        m_images(Renderer::WorkBuffer()),
        m_imageViews(Renderer::WorkBuffer()),
        m_swapChainBuffers(Renderer::WorkBuffer())
    {

    }

    VkSurfaceFormatKHR ChoseSurfaceFormat(const dynamic_vector<VkSurfaceFormatKHR>& formats)
    {
        for(const VkSurfaceFormatKHR& format : formats)
        {
            if(format.format == VK_FORMAT_B8G8R8A8_SRGB &&
               format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
            {
                return format;
            }
        }

        return formats[0];
    }

    VkPresentModeKHR ChosePresentMode(const dynamic_vector<VkPresentModeKHR>& present_modes)
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

        uint32_t queue_family_indices_array[] = {indices.GraphicsFamilyIndex.value(),
                                                 indices.PresentationFamilyIndex.value()};

        if(indices.GraphicsFamilyIndex != indices.PresentationFamilyIndex)
        {
            create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            create_info.queueFamilyIndexCount = 2;
            create_info.pQueueFamilyIndices = queue_family_indices_array;
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
            vkCreateSwapchainKHR(logicalDevice, &create_info, nullptr, &m_swapChain);

        SJ_ASSERT(VkResult::VK_SUCCESS == swap_chain_create_success, "Failed to create swapchain");

        // Extract swap chain image handles
        uint32_t real_image_count;
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
            VkImageViewCreateInfo image_view_create_info {};
            image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_create_info.image = m_images[i];
            image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_create_info.format = m_chainImageFormat;

            image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_view_create_info.subresourceRange.baseMipLevel = 0;
            image_view_create_info.subresourceRange.levelCount = 1;
            image_view_create_info.subresourceRange.baseArrayLayer = 0;
            image_view_create_info.subresourceRange.layerCount = 1;

            VkResult view_create_success = vkCreateImageView(logicalDevice,
                                                             &image_view_create_info,
                                                             nullptr,
                                                             &m_imageViews[i]);

            SJ_ASSERT(view_create_success == VkResult::VK_SUCCESS,
                      "Failed to create swap chain image view");
        }

    }

    void VulkanSwapChain::InitFrameBuffers(VkDevice device, VkRenderPass pass)
    {

        // Create Frame Buffers
        m_swapChainBuffers.resize(m_imageViews.size());

        int i = 0;
        for(VkImageView& view : m_imageViews)
        {

            VkFramebufferCreateInfo framebufferInfo {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = pass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &view;
            framebufferInfo.width = GetExtent().width;
            framebufferInfo.height = GetExtent().height;
            framebufferInfo.layers = 1;

            VkResult res = vkCreateFramebuffer(device,
                                               &framebufferInfo,
                                               nullptr,
                                               &m_swapChainBuffers[i]);

            SJ_ASSERT(res == VK_SUCCESS, "Failed to construct frame buffers.");

            i++;
        }
    }

    void VulkanSwapChain::DeInit(VkDevice logicalDevice)
    {
        for(VkFramebuffer buffer : m_swapChainBuffers)
        {
            vkDestroyFramebuffer(logicalDevice, buffer, nullptr);
        }

        for(VkImageView view : m_imageViews)
        {
            vkDestroyImageView(logicalDevice, view, nullptr);
        }

        vkDestroySwapchainKHR(logicalDevice, m_swapChain, nullptr);
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

    VulkanSwapChain::SwapChainParams
    VulkanSwapChain::QuerySwapChainParams(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
    {
        SwapChainParams params {};

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device,
                                                  surface,
                                                  &params.Capabilities);

        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
                                             &format_count,
                                             nullptr);

        SJ_ASSERT(format_count != 0, "No surface formats found");

        params.Formats.reserve(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,
                                             surface,
                                             &format_count,
                                             params.Formats.data());

        uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
                                                  surface,
                                                  &present_mode_count,
                                                  nullptr);

        SJ_ASSERT(present_mode_count != 0, "No present modes found");
        params.PresentModes.reserve(present_mode_count);
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