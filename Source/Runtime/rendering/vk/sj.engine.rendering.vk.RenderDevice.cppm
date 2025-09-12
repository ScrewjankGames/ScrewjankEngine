module;
#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/PlatformDetection.hpp>

#include <vulkan/vulkan_core.h>
#include <VkBootstrap.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

export module sj.engine.rendering.vk.RenderDevice;
export import sj.engine.rendering.vk.Utils;
import sj.std.containers.array;
import sj.std.containers.set;
import sj.std.containers.vector;
import sj.std.memory;
import sj.engine.system.threading.ThreadContext;
import sj.engine.rendering.vk.Primitives;

export namespace sj::vk
{
    class RenderDevice
    {
    public:
        RenderDevice() = default;
        ~RenderDevice() = default;

        void Init(const vkb::Instance& instanceInfo, VkSurfaceKHR renderSurface)
        {
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

            VkPhysicalDeviceFeatures features 
            {
                .samplerAnisotropy = VK_TRUE 
            };

            // use vkbootstrap to select a gpu.
            vkb::PhysicalDeviceSelector selector {instanceInfo};
            vkb::PhysicalDevice physicalDevice = selector.set_minimum_version(1, 3)
                                                     .set_required_features_13(features1_3)
                                                     .set_required_features_12(features1_2)
                                                     .set_required_features(features)
                                                     .set_surface(renderSurface)
                                                     .select()
                                                     .value();

            // create the final vulkan device
            vkb::DeviceBuilder deviceBuilder {physicalDevice};
            vkb::Device vkbDevice = deviceBuilder.build().value();

            // Get the VkDevice handle used in the rest of a vulkan application
            m_logicalDevice = vkbDevice.device;
            m_physicalDevice = physicalDevice.physical_device;

            VmaAllocatorCreateInfo allocatorInfo = {};
            allocatorInfo.physicalDevice = m_physicalDevice;
            allocatorInfo.device = m_logicalDevice;
            allocatorInfo.instance = instanceInfo.instance;
            allocatorInfo.pAllocationCallbacks = sj::g_vkAllocationFns;
            allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
            vmaCreateAllocator(&allocatorInfo, &m_allocator);

            SJ_ENGINE_LOG_INFO("Selected GPU {}", vkbDevice.physical_device.name.c_str());

            // Grab graphics queue info
            m_graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
            m_graphicsQueueIndex =
                vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

            m_presentationQueue = vkbDevice.get_queue(vkb::QueueType::present).value();
            m_presentationQueueIndex = vkbDevice.get_queue_index(vkb::QueueType::present).value(); 
        }

        void DeInit()
        {
            vmaDestroyAllocator(m_allocator);

            if(m_logicalDevice != VK_NULL_HANDLE)
            {
                vkDestroyDevice(m_logicalDevice, sj::g_vkAllocationFns);
            }
        }

        /**
         * @return The vulkan handle for the logical device
         */
        [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const
        {
            return m_physicalDevice;
        }

        /**
         * @return The vulkan handle for the logical device
         */
        [[nodiscard]] VkDevice GetLogicalDevice() const
        {
            return m_logicalDevice;
        }

        [[nodiscard]] VmaAllocator GetAllocator() const
        {
            return m_allocator;
        }

        [[nodiscard]] VkQueue GetGraphicsQueue() const
        {
            return m_graphicsQueue;
        }

        [[nodiscard]] uint32_t GetGraphicsQueueIndex() const
        {
            return m_graphicsQueueIndex;
        }

        [[nodiscard]] VkQueue GetPresentationQueue() const
        {
            return m_presentationQueue;
        }

        [[nodiscard]] uint32_t GetPresentationQueueIndex() const
        {
            return m_presentationQueueIndex;
        }

    private:
        /** Vulkan's logical representation of the physical device */
        VkDevice m_logicalDevice = VK_NULL_HANDLE;

        /**
         * Vulkan's representation of the physical rendering device
         * @note This handle is freed automatically when the VkInstance is destroyed
         */
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

        /** Manages device's memory */
        VmaAllocator m_allocator;

        /** Queue used to handle graphics commands */
        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        uint32_t m_graphicsQueueIndex = -1;

        /** Used to execute presentation commands */
        VkQueue m_presentationQueue = VK_NULL_HANDLE;
        uint32_t m_presentationQueueIndex = -1;
    };
} // namespace sj::vk