module;
#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/PlatformDetection.hpp>

#include <vulkan/vulkan_core.h>

#include <array>
#include <optional>

export module sj.engine.rendering.vk.RenderDevice;
import sj.std.containers.array;
import sj.std.containers.set;
import sj.std.containers.vector;
import sj.std.memory;
import sj.engine.rendering.vk.Utils;
import sj.engine.system.threading.ThreadContext;

/** List of extensions devices must support */
static constexpr std::array<const char*, 1> kRequiredDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

export namespace sj::vk
{
    class RenderDevice
    {
    public:
        RenderDevice() = default;
        ~RenderDevice() = default;

        void Init(VkInstance instance, VkSurfaceKHR renderSurface)
        {
            SelectPhysicalDevice(instance, renderSurface);
            CreateLogicalDevice(renderSurface);
        }

        void DeInit()
        {
            if(m_Device != VK_NULL_HANDLE)
            {
                vkDestroyDevice(m_Device, sj::g_vkAllocationFns);
            }
        }

        /**
         * @return The vulkan handle for the logical device
         */
        [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const
        {
            return m_PhysicalDevice;
        }

        /**
         * @return The vulkan handle for the logical device
         */
        [[nodiscard]] VkDevice GetLogicalDevice() const
        {
            return m_Device;
        }

        [[nodiscard]] VkQueue GetGraphicsQueue() const
        {
            return m_GraphicsQueue;
        }

        [[nodiscard]] VkQueue GetPresentationQueue() const
        {
            return m_PresentationQueue;
        }

    private:
        /**
         * Iterates over the system's rendering hardware, and selects the most suitable GPU for
         * rendering
         */
        void SelectPhysicalDevice(VkInstance instance, VkSurfaceKHR renderSurface)
        {
            uint32_t deviceCount = 0;

            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
            SJ_ENGINE_LOG_INFO("{} Vulkan-capable render devices detected", deviceCount);

            scratchpad_scope scratchpad = ThreadContext::GetScratchpad();
            dynamic_array<VkPhysicalDevice> devices(deviceCount, &scratchpad.get_allocator());
            vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

            int best_score = -1;

            // Picks the best compatible GPU found, prefering discrete GPUs
            for(const VkPhysicalDevice& device : devices)
            {
                int score = 0;

                if(!IsDeviceSuitable(device, renderSurface))
                {
                    continue;
                }

                VkPhysicalDeviceProperties deviceProps;
                VkPhysicalDeviceFeatures deviceFeatures;

                vkGetPhysicalDeviceProperties(device, &deviceProps);
                vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

                if(deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                {
                    score += 1000;
                }
                else if(deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
                {
                    score += 500;
                }

                if(score > best_score)
                {
                    best_score = score;
                    m_PhysicalDevice = device;
                }
            }

            if constexpr(g_IsDebugBuild)
            {
                VkPhysicalDeviceProperties device_props;
                vkGetPhysicalDeviceProperties(m_PhysicalDevice, &device_props);

                SJ_ENGINE_LOG_INFO("Selected render device: {}", device_props.deviceName);
            }

            SJ_ASSERT(m_PhysicalDevice != VK_NULL_HANDLE,
                      "Screwjank Engine failed to select suitable physical device.");
        }

        /**
         * After selecting a physical device, construct the corresponding logical device
         */
        void CreateLogicalDevice(VkSurfaceKHR renderSurface)
        {
            SJ_ASSERT(m_PhysicalDevice != VK_NULL_HANDLE,
                      "CreateLogicalDevice requires a selected physical device");
            SJ_ASSERT(m_Device == VK_NULL_HANDLE, "Logical device already created.");

            DeviceQueueFamilyIndices indices =
                GetDeviceQueueFamilyIndices(m_PhysicalDevice, renderSurface);

            constexpr int kMaxUniqueQueues = 2;
            static_set<uint32_t, kMaxUniqueQueues> unique_queue_families;
            unique_queue_families = {
                indices.graphicsFamilyIndex.value(),
                indices.presentationFamilyIndex.value(),
            };

            static_vector<VkDeviceQueueCreateInfo, kMaxUniqueQueues> queue_create_infos;
            float queue_priorities = 1.0f;
            for(auto family : unique_queue_families)
            {
                VkDeviceQueueCreateInfo queue_create_info {};
                queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queue_create_info.queueFamilyIndex = family;
                queue_create_info.queueCount = 1;
                queue_create_info.pQueuePriorities = &queue_priorities;
                queue_create_infos.emplace_back(queue_create_info);
            }

            VkPhysicalDeviceFeatures device_features = {};
            device_features.samplerAnisotropy = VK_TRUE;

            VkDeviceCreateInfo device_create_info = {};
            device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            device_create_info.queueCreateInfoCount =
                static_cast<uint32_t>(queue_create_infos.size());
            device_create_info.pQueueCreateInfos = queue_create_infos.data();
            device_create_info.enabledLayerCount = 0;
            device_create_info.pEnabledFeatures = &device_features;

            const char* const* deviceExtensionNames = kRequiredDeviceExtensions.data();
            device_create_info.enabledExtensionCount =
                static_cast<uint32_t>(kRequiredDeviceExtensions.size());
            device_create_info.ppEnabledExtensionNames = deviceExtensionNames;

            VkResult success = vkCreateDevice(m_PhysicalDevice,
                                              &device_create_info,
                                              sj::g_vkAllocationFns,
                                              &m_Device);

            SJ_ASSERT(success == VK_SUCCESS, "Vulkan failed to create logical device.");

            vkGetDeviceQueue(m_Device, *indices.graphicsFamilyIndex, 0, &m_GraphicsQueue);
            vkGetDeviceQueue(m_Device, *indices.presentationFamilyIndex, 0, &m_PresentationQueue);
        }

        /**
         * Queries physical device suitability for use in engine
         */
        static bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR renderSurface)
        {
            DeviceQueueFamilyIndices indices = GetDeviceQueueFamilyIndices(device, renderSurface);

            // Query queue support
            bool indicies_complete = indices.graphicsFamilyIndex.has_value() &&
                                     indices.presentationFamilyIndex.has_value();

            static_set<std::string_view, kRequiredDeviceExtensions.size()> missing_extensions(
                kRequiredDeviceExtensions.begin(),
                kRequiredDeviceExtensions.end());

            // Check extension support
            {
                uint32_t extension_count = 0;
                vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

                dynamic_array<VkExtensionProperties> extension_props(extension_count);

                vkEnumerateDeviceExtensionProperties(device,
                                                     nullptr,
                                                     &extension_count,
                                                     extension_props.data());

                for(const VkExtensionProperties& extension : extension_props)
                {
                    missing_extensions.erase(extension.extensionName);
                }
            }

            // Check swap chain support
            SwapChainParams params = QuerySwapChainParams(device, renderSurface);
            bool swap_chain_supported =
                (params.Formats.size() > 0) && (params.PresentModes.size() > 0);

            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

            return indicies_complete && missing_extensions.size() == 0 && swap_chain_supported &&
                   supportedFeatures.samplerAnisotropy;
        }

        /** Vulkan's logical representation of the physical device */
        VkDevice m_Device = VK_NULL_HANDLE;

        /** Queue used to handle graphics commands */
        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;

        /** Used to execute presentation commands */
        VkQueue m_PresentationQueue = VK_NULL_HANDLE;

        /**
         * Vulkan's representation of the physical rendering device
         * @note This handle is freed automatically when the VkInstance is destroyed
         */
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    };
} // namespace sj::vk