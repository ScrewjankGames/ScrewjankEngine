module;

// Screwjank Headers
#include <ScrewjankStd/Log.hpp>
#include <ScrewjankStd/Assert.hpp>

// Library Headers
#include <vulkan/vulkan.h>

// STD Headers
#include <ranges>

export module sj.engine.rendering.vk.Utils;
import sj.datadefs.assets.Mesh;
import sj.std.containers.vector;
import sj.std.containers.array;
import sj.engine.system.memory;
import sj.engine.system.threading.ThreadContext;
import sj.engine.rendering.vk.Primitives;

export namespace sj
{
    [[nodiscard]]
    VkImageView
    CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
    {
        VkImageViewCreateInfo viewInfo {};

        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView {};
        VkResult res = vkCreateImageView(device, &viewInfo, sj::g_vkAllocationFns, &imageView);
        SJ_ASSERT(res == VK_SUCCESS, "Failed to create texture image view");

        return imageView;
    }

    /**
     * Query swap chain support parameters
     */
    struct SwapChainParams
    {
        VkSurfaceCapabilitiesKHR Capabilities;
        sj::static_vector<VkSurfaceFormatKHR, 32> Formats;
        sj::static_vector<VkPresentModeKHR, 32> PresentModes;
    };
    SwapChainParams QuerySwapChainParams(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
    {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

        SwapChainParams params {
            .Capabilities = capabilities,
        };

        uint32_t format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
        SJ_ASSERT(format_count != 0, "No surface formats found");
        params.Formats.resize(format_count);

        uint32_t present_mode_count = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
                                                  surface,
                                                  &present_mode_count,
                                                  nullptr);
        SJ_ASSERT(present_mode_count != 0, "No present modes found");
        params.PresentModes.resize(present_mode_count);

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

    void CheckImguiVulkanResult(VkResult res)
    {
        SJ_ASSERT(res == VK_SUCCESS, "ImGui Vulkan operation failed!");
    }

} // namespace sj
