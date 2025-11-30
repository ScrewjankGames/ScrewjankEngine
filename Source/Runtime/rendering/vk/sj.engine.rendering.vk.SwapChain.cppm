module;

// STD Headers
#include <span>
#include <vector>

// Library Headers
#include <vk_mem_alloc.h>

// Screwjank Headers
#include <ScrewjankStd/Assert.hpp>

export module sj.engine.rendering.vk.SwapChain;
import sj.engine.rendering.vk.Utils;
import sj.engine.rendering.vk.ImageUtils;
import sj.engine.rendering.vk.Primitives;
import sj.engine.rendering.vk.RenderDevice;

import sj.std.containers.array;
import sj.std.containers.vector;

import sj.std.memory;
import sj.std.math;

import vulkan_hpp;

namespace sj::vulkan
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

} // namespace sj::vulkan

export namespace sj::vulkan
{
class SwapChain
{
public:
    SwapChain() = default;
    ~SwapChain() = default;

    void Init(sj::vulkan::RenderDevice& device, VkSurfaceKHR renderingSurface, sj::Vec2 surfaceSize)
    {
        SJ_ASSERT(!m_isInitialized, "Double initialization detected!");

        SwapChainParams params = QuerySwapChainParams(*device.mPhysicalDevice, renderingSurface);

        VkSurfaceFormatKHR selected_format = ChoseSurfaceFormat(params.Formats);
        VkPresentModeKHR present_mode = ChosePresentMode(params.PresentModes);
        VkExtent2D extent = QuerySwapExtent(surfaceSize, params.Capabilities);

        SJ_ASSERT(selected_format.format != VK_FORMAT_UNDEFINED,
                  "Failed to select swap chain surface format");

        uint32_t image_count = params.Capabilities.minImageCount + 1;
        if(params.Capabilities.maxImageCount > 0 && image_count > params.Capabilities.maxImageCount)
        {
            image_count = params.Capabilities.maxImageCount;
        }

        std::array queue_family_indices_array = {device.mGraphicsQueueIndex,
                                                 device.mPresentationQueueIndex};
        vk::SharingMode sharingMode = vk::SharingMode::eConcurrent;
        uint32_t queueFamilyIndexCount = queue_family_indices_array.size();
        uint32_t* queueFamilyIndicesPtr = queue_family_indices_array.data();
        if(device.mGraphicsQueueIndex == device.mPresentationQueueIndex)
        {
            sharingMode = vk::SharingMode::eExclusive;
            queueFamilyIndexCount = 0;
            queueFamilyIndicesPtr = nullptr;
        }

        m_chainImageFormat = static_cast<vk::Format>(selected_format.format);

        vk::SwapchainCreateInfoKHR create_info(
            vk::SwapchainCreateFlagsKHR {},
            renderingSurface,
            image_count,
            m_chainImageFormat,
            static_cast<vk::ColorSpaceKHR>(selected_format.colorSpace),
            extent,
            1,
            vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
            sharingMode,
            queueFamilyIndexCount,
            queueFamilyIndicesPtr,
            static_cast<vk::SurfaceTransformFlagBitsKHR>(params.Capabilities.currentTransform),
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            static_cast<vk::PresentModeKHR>(present_mode),
            true,
            mSwapChain);

        mSwapChain =
            vk::raii::SwapchainKHR(device.mLogicalDevice, create_info, g_vkAllocationCallbacks);

        // Extract swap chain image handles
        m_images = mSwapChain.getImages();
        m_imageExtent = extent;

        // Create Image Views
        m_imageViews.reserve(m_images.size());
        for(VkImage image : m_images)
        {
            m_imageViews.emplace_back(CreateImageView(device.mLogicalDevice,
                                                      image,
                                                      m_chainImageFormat,
                                                      vk::ImageAspectFlagBits::eColor));
        }

        vk::SemaphoreCreateInfo semaphoreInfo;
        m_renderFinishedSemaphores.reserve(m_images.size());
        for(size_t i = 0; i < m_images.size(); i++)
        {
            m_renderFinishedSemaphores.emplace_back(
                vk::raii::Semaphore(device.mLogicalDevice, semaphoreInfo, g_vkAllocationCallbacks));
        }
    }

    void
    Recreate(sj::vulkan::RenderDevice& device, VkSurfaceKHR renderingSurface, sj::Vec2 surfaceSize)
    {
        // Window is minimized. Can't recreate swap chain.
        if((surfaceSize.GetX() * surfaceSize.GetY()) == 0)
            return;

        vkDeviceWaitIdle(*device.mLogicalDevice);

        m_renderFinishedSemaphores.clear();
        m_imageViews.clear();

        Init(device, renderingSurface, surfaceSize);
    }

    [[nodiscard]] VkImage GetImage(uint32_t idx)
    {
        SJ_ASSERT(idx >= 0 && idx < m_images.size(), "Invalid index");
        return m_images[idx];
    }

    [[nodiscard]] VkImageView GetImageView(uint32_t idx)
    {
        SJ_ASSERT(idx >= 0 && idx < m_imageViews.size(), "Invalid index");
        return *m_imageViews[idx];
    }

    [[nodiscard]] VkExtent2D GetExtent() const
    {
        return m_imageExtent;
    }

    [[nodiscard]] vk::Format GetImageFormat() const
    {
        return m_chainImageFormat;
    }

    [[nodiscard]] VkSwapchainKHR GetSwapChain() const
    {
        return *mSwapChain;
    }

    [[nodiscard]] std::span<vk::raii::ImageView> GetImageViews() const
    {
        return m_imageViews;
    }

    [[nodiscard]] VkSemaphore GetImageRenderCompleteSemaphore(uint32_t imageIdx)
    {
        return *m_renderFinishedSemaphores.at(imageIdx);
    }

private:
    /**
     * Communicates with the window to query swap chain extents
     */
    VkExtent2D QuerySwapExtent(sj::Vec2 surfaceExtent, const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }

        VkExtent2D extent {static_cast<uint32_t>(surfaceExtent.GetX()),
                           static_cast<uint32_t>(surfaceExtent.GetY())};

        // Clamp values to max supported by implementation
        extent.width = std::max(capabilities.minImageExtent.width,
                                std::min(capabilities.maxImageExtent.width, extent.width));

        extent.height = std::max(capabilities.minImageExtent.height,
                                 std::min(capabilities.maxImageExtent.height, extent.height));

        return extent;
    }

    bool m_isInitialized = false;

    vk::raii::SwapchainKHR mSwapChain {nullptr};
    std::vector<vk::Image> m_images;
    dynamic_vector<vk::raii::ImageView> m_imageViews;

    /** Semaphores to track when each image is presented  */
    dynamic_vector<vk::raii::Semaphore> m_renderFinishedSemaphores;

    vk::Format m_chainImageFormat {};
    VkExtent2D m_imageExtent = {};
};

} // namespace sj::vulkan