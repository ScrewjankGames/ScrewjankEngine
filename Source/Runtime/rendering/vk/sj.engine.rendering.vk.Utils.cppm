module;

// Screwjank Headers
#include <ScrewjankDataDefinitions/Assets/Model.hpp>
#include <ScrewjankStd/Log.hpp>
#include <ScrewjankStd/Assert.hpp>

// Library Headers
#include <vulkan/vulkan.h>

export module sj.engine.rendering.vk.Utils;
import sj.std.containers.vector;
import sj.std.containers.array;
import sj.engine.system.memory;
import sj.engine.system.threading.ThreadContext;

export namespace sj
{
    inline constexpr VkAllocationCallbacks* g_vkAllocationFns = nullptr;

    [[nodiscard]]
    uint32_t FindMemoryType(VkPhysicalDevice physicalDevice,
                            uint32_t typeFilter,
                            VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if((typeFilter & (1 << i)) &&
               (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        SJ_ASSERT(false, "Failed to find suitable memory type.");
        return -1;
    }

    void CreateImage(VkDevice logicalDevice,
                     VkPhysicalDevice physicalDevice,
                     uint32_t width,
                     uint32_t height,
                     VkFormat format,
                     VkImageTiling tiling,
                     VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties,
                     VkImage& image,
                     VkDeviceMemory& imageMemory)
    {
        VkImageCreateInfo imageInfo {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult success = vkCreateImage(logicalDevice, &imageInfo, sj::g_vkAllocationFns, &image);

        SJ_ASSERT(success == VK_SUCCESS, "failed to create image!");

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex =
            FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

        success = vkAllocateMemory(logicalDevice, &allocInfo, sj::g_vkAllocationFns, &imageMemory);
        SJ_ASSERT(success == VK_SUCCESS, "failed to allocate image memory!");

        vkBindImageMemory(logicalDevice, image, imageMemory, 0);
    }

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
        desc.stride = sizeof(Vertex);
        desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return desc;
    }

    inline std::array<VkVertexInputAttributeDescription, 3> GetVertexAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, uv);

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

    /**
     * Returns the queue families available for the supplied VkPhysicalDevice
     */
    struct DeviceQueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamilyIndex;
        std::optional<uint32_t> presentationFamilyIndex;
    };

    DeviceQueueFamilyIndices
    GetDeviceQueueFamilyIndices(VkPhysicalDevice device,
                                VkSurfaceKHR renderSurface = VK_NULL_HANDLE)
    {
        uint32_t queue_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, nullptr);
        scratchpad_scope scratchpad = ThreadContext::GetScratchpad();
        dynamic_array<VkQueueFamilyProperties> queue_data(queue_count,
                                                          &scratchpad.get_allocator());
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, queue_data.data());

        DeviceQueueFamilyIndices indices;

        int i = 0;
        for(const VkQueueFamilyProperties& family : queue_data)
        {
            if(family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamilyIndex = i;
            }

            if(renderSurface != VK_NULL_HANDLE)
            {
                VkBool32 presentation_support = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device,
                                                     i,
                                                     renderSurface,
                                                     &presentation_support);

                if(presentation_support)
                {
                    indices.presentationFamilyIndex = i;
                }
            }

            i++;
        }

        return indices;
    }
} // namespace sj
