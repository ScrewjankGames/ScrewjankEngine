module;

// Screwjank Headers
#include <ScrewjankStd/Log.hpp>
#include <ScrewjankStd/Assert.hpp>

// Library Headers
#include <vulkan/vulkan.h>

// STD Headers
#include <memory_resource>
#include <ranges>

export module sj.engine.rendering.vk.Utils;
import sj.datadefs.assets.Mesh;
import sj.std.containers.vector;
import sj.std.containers.array;
import sj.engine.system.memory;
import sj.engine.system.threading.ThreadContext;
import sj.engine.rendering.vk.Primitives;

import vulkan_hpp;

export namespace sj
{
[[nodiscard]]
vk::raii::ImageView CreateImageView(vk::raii::Device& device,
                                    VkImage image,
                                    vk::Format format,
                                    vk::ImageAspectFlagBits aspectFlags)
{
    vk::ImageViewCreateInfo viewInfo({},
                                     image,
                                     vk::ImageViewType::e2D,
                                     format,
                                     {},
                                     vk::ImageSubresourceRange(aspectFlags, 0, 1, 0, 1));

    return vk::raii::ImageView(device, viewInfo, g_vkAllocationCallbacks);
}

/**
 * Query swap chain support parameters
 */
struct SwapChainParams
{
    VkSurfaceCapabilitiesKHR Capabilities;
    sj::dynamic_array<VkSurfaceFormatKHR> Formats;
    sj::dynamic_array<VkPresentModeKHR> PresentModes;
};
SwapChainParams QuerySwapChainParams(std::pmr::memory_resource* resource, VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

    SwapChainParams params {
        .Capabilities = capabilities,
        .Formats = sj::dynamic_array<VkSurfaceFormatKHR>(resource),
        .PresentModes = sj::dynamic_array<VkPresentModeKHR>(resource),
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
