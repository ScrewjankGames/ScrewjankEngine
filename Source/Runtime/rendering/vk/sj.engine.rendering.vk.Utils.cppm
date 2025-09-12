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

    [[nodiscard]]
    VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice,
                                 std::span<VkFormat> candidates,
                                 VkImageTiling tiling,
                                 VkFormatFeatureFlags features)
    {
        for(VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if(tiling == VK_IMAGE_TILING_LINEAR &&
               (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if(tiling == VK_IMAGE_TILING_OPTIMAL &&
                    (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        SJ_ASSERT(false, "Failed to acquire supported VkFormat with given arguments");
        return VK_FORMAT_UNDEFINED;
    }

    [[nodiscard]]
    VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice)
    {
        std::array depthFormats = {VK_FORMAT_D32_SFLOAT,
                                   VK_FORMAT_D32_SFLOAT_S8_UINT,
                                   VK_FORMAT_D24_UNORM_S8_UINT};

        VkFormat depthFormat = FindSupportedFormat(physicalDevice,
                                                   depthFormats,
                                                   VK_IMAGE_TILING_OPTIMAL,
                                                   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        return depthFormat;
    }

    inline VkVertexInputBindingDescription GetVertexBindingDescription()
    {
        VkVertexInputBindingDescription desc {};

        desc.binding = 0;
        desc.stride = sizeof(MeshVertex);
        desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return desc;
    }

    inline std::ranges::range auto GetVertexAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(MeshVertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(MeshVertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(MeshVertex, uv);

        return attributeDescriptions;
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

    bool HasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    /**
     * Callback function that allows the Vulkan API to use the engine's logging system
     * @note See Vulkan API for description of arguments
     */
    VKAPI_ATTR VkBool32 VKAPI_CALL
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

} // namespace sj
