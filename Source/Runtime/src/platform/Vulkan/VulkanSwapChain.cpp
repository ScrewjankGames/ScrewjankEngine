// STD Headers

// Library Headers

// Screwjank Headers
#include <platform/Vulkan/VulkanSwapChain.hpp>
#include <core/Window.hpp>
#include <platform/Vulkan/VulkanRendererAPI.hpp>
#include <platform/Vulkan/VulkanRenderDevice.hpp>

namespace sj
{
    VulkanSwapChain::VulkanSwapChain(VulkanRendererAPI* api, Window* target_window)
        : m_API(api), m_TargetWindow(target_window), m_SwapChain(VK_NULL_HANDLE), m_ChainImageFormat(VkFormat::VK_FORMAT_UNDEFINED),
          m_ImageExtent()
    {
        SwapChainParams params = QuerySwapChainParams(m_API->GetPhysicalDevice(), m_API->GetRenderingSurface());
        InitializeSwapChain(params);
    }

    VulkanSwapChain::~VulkanSwapChain()
    {
        for (auto view : m_ImageViews)
        {
            vkDestroyImageView(m_API->GetLogicalDevice(), view, nullptr);
        }

        vkDestroySwapchainKHR(m_API->GetLogicalDevice(), m_SwapChain, nullptr);
    }

    VkExtent2D VulkanSwapChain::QuerySwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }

        Window::FrameBufferSize frame_buffer_size = m_TargetWindow->GetFrameBufferSize();
        VkExtent2D extent {frame_buffer_size.Width, frame_buffer_size.Height};

        // Clamp values to max supported by implementation
        extent.width = std::max(capabilities.minImageExtent.width,
                                std::min(capabilities.maxImageExtent.width, extent.width));

        extent.height = std::max(capabilities.minImageExtent.height,
                                 std::min(capabilities.maxImageExtent.height, extent.height));

        return extent;
    }

    VulkanSwapChain::SwapChainParams
    VulkanSwapChain::QuerySwapChainParams(VkPhysicalDevice physical_device, VkSurfaceKHR surface) const
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

        params.Formats.Reserve(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,
                                             surface,
                                             &format_count,
                                             params.Formats.Data());

        uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
                                                  surface,
                                                  &present_mode_count,
                                                  nullptr);

        SJ_ASSERT(present_mode_count != 0, "No present modes found");
        params.PresentModes.Reserve(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
                                                  surface,
                                                  &present_mode_count,
                                                  params.PresentModes.Data());

        return params;
    }

    VkSurfaceFormatKHR ChoseSurfaceFormat(const Vector<VkSurfaceFormatKHR>& formats)
    {
        for (const VkSurfaceFormatKHR& format : formats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
                format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
            {
                return format;
            }
        }

        return formats[0];
    }

    VkPresentModeKHR ChosePresentMode(const Vector<VkPresentModeKHR>& present_modes)
    {
        for (const VkPresentModeKHR& mode : present_modes)
        {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return mode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    void VulkanSwapChain::InitializeSwapChain(const SwapChainParams& params)
    {

        VkSurfaceFormatKHR selected_format = ChoseSurfaceFormat(params.Formats);
        VkPresentModeKHR present_mode = ChosePresentMode(params.PresentModes);
        VkExtent2D extent = QuerySwapExtent(params.Capabilities);

        SJ_ASSERT(selected_format.format != VK_FORMAT_UNDEFINED,
                  "Failed to select swap chain surface format");

        uint32_t image_count = params.Capabilities.minImageCount + 1;
        if (params.Capabilities.maxImageCount > 0 &&
            image_count > params.Capabilities.maxImageCount)
        {
            image_count = params.Capabilities.maxImageCount;
        }

        // Fill out swap chain create info
        VkSwapchainCreateInfoKHR create_info {};
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface = m_API->GetRenderingSurface();
        create_info.minImageCount = image_count;
        create_info.imageFormat = selected_format.format;
        create_info.imageColorSpace = selected_format.colorSpace;
        create_info.imageExtent = extent;
        create_info.imageArrayLayers = 1;
        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        DeviceQueueFamilyIndices indices =
            m_API->GetDeviceQueueFamilyIndices(m_API->GetPhysicalDevice());

        uint32_t queue_family_indices_array[] = {indices.GraphicsFamilyIndex.Value(),
                                                 indices.PresentationFamilyIndex.Value()};

        if (indices.GraphicsFamilyIndex != indices.PresentationFamilyIndex)
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
        VkResult swap_chain_create_success = vkCreateSwapchainKHR(m_API->GetLogicalDevice(),
                             &create_info,
                             nullptr,
                             &m_SwapChain);

        SJ_ASSERT(VkResult::VK_SUCCESS == swap_chain_create_success, "Failed to create swapchain");

        // Extract swap chain image handles
        uint32_t real_image_count;
        vkGetSwapchainImagesKHR(m_API->GetLogicalDevice(),
                                m_SwapChain,
                                &real_image_count,
                                nullptr);

        m_Images.Resize(real_image_count);

        vkGetSwapchainImagesKHR(m_API->GetLogicalDevice(),
                                m_SwapChain,
                                &real_image_count,
                                m_Images.Data());

        m_ChainImageFormat = selected_format.format;
        m_ImageExtent = extent;

        // Create Image Views
        m_ImageViews.Resize(m_Images.Size());

        for (size_t i = 0; i < m_Images.Size(); i++)
        {
            VkImageViewCreateInfo image_view_create_info {};
            image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_create_info.image = m_Images[i];
            image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_create_info.format = m_ChainImageFormat;
            
            image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            
            image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_view_create_info.subresourceRange.baseMipLevel = 0;
            image_view_create_info.subresourceRange.levelCount = 1;
            image_view_create_info.subresourceRange.baseArrayLayer = 0;
            image_view_create_info.subresourceRange.layerCount = 1;

            VkResult view_create_success = vkCreateImageView(m_API->GetLogicalDevice(),
                              &image_view_create_info,
                              nullptr,
                              &m_ImageViews[i]);

            SJ_ASSERT(view_create_success == VkResult::VK_SUCCESS,
                      "Failed to create swap chain image view");
        }

    }

} // namespace sj