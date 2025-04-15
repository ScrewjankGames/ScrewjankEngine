#pragma once

// Screwjank Headers
#include <ScrewjankEngine/system/memory/Memory.hpp>
#include <ScrewjankEngine/containers/String.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanRenderDevice.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanSwapChain.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanPipeline.hpp>

// Shared Headers
#include <ScrewjankShared/Math/Vec2.hpp>
#include <ScrewjankShared/Math/Vec3.hpp>
#include <ScrewjankShared/Math/Mat44.hpp>

// Library Headers
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

// STD Headers
#include <span>

import sj.shared.containers;

namespace sj {
    // Forward declarations
    class VulkanPipeline;
    class VulkanRenderDevice;
    class VulkanSwapChain;
    class Window;
    struct DeviceQueueFamilyIndices;
    
    class Renderer
    {
    public:
        /** Allocator used for engine helper allocations to get data over to vulkan */
        static MemSpace<FreeListAllocator>* WorkBuffer();

        static Renderer* GetInstance();

        void Init();
        void DeInit();

        void StartRenderFrame();
        void Render(const Mat44& cameraMatrix);

        VkInstance GetVkInstanceHandle() const;

        /**
         * Global state shared by all shaders
         */
        struct GlobalUniformBufferObject
        {
            Mat44 model;
            Mat44 view;
            Mat44 projection;
        };

      private:
        Renderer() = default;
        ~Renderer() = default;

        static constexpr uint32_t kMaxFramesInFlight = 2;

        /**
         * Callback function that allows the Vulkan API to use the engine's logging system
         * @note See Vulkan API for description of arguments
         */
        static VKAPI_ATTR VkBool32 VKAPI_CALL
        VulkanDebugLogCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                               VkDebugUtilsMessageTypeFlagsEXT message_type,
                               const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                               void* user_data);

        static bool HasStencilComponent(VkFormat format);

        /**
         * Initializes the Vulkan API's instance and debug messaging hooks
         */
        void InitializeVulkan();

        /**
         * Communicates with the Window to creates the rendering surface
         */
        void CreateRenderSurface();

        void CreateRenderPass();

        VkCommandBuffer BeginSingleTimeCommands();
        void EndSingleTimeCommands(VkCommandBuffer& commandBuffer);

        void EnableValidationLayers(std::span<const char*> required_validation_layers);

        void EnableDebugMessaging();

        void TransitionImageLayout(VkImage image,
                                   VkImageLayout oldLayout,
                                   VkImageLayout newLayout);

        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

        void CreateCommandPools();

        void CreateBuffer(VkDeviceSize size,
                          VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties,
                          VkBuffer& out_buffer,
                          VkDeviceMemory& out_bufferMemory);

        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        void LoadDummyModel();

        void CreateDummyTextureImage();
        void CreateDummyTextureSampler();

        void CreateGlobalDescriptorSetlayout();
        void CreateGlobalUniformBuffers();

        void CreateGlobalUBODescriptorPool();

        #ifndef SJ_GOLD
        void CreateImGuiDescriptorPool();
        #endif // !SJ_GOLD


        void CreateGlobalUBODescriptorSets();
        
        void RecordCommandBuffer(VkCommandBuffer buffer, uint32_t frameIdx, uint32_t imageIdx);

        void UpdateUniformBuffer(void* bufferMem, const Mat44& cameraMatrix);

        /** The Vulkan instance is the engine's connection to the vulkan library */
        VkInstance m_vkInstance;

        /** Handle to manage Vulkan's debug callbacks */
        VkDebugUtilsMessengerEXT m_vkDebugMessenger;

        /** Handle to the surface vulkan renders to */
        VkSurfaceKHR m_renderingSurface;

        /** It's a render pass I guess. */
        VkRenderPass m_defaultRenderPass;

        /** Used to back API operations */
        VulkanRenderDevice m_renderDevice;

        /** Used for image presentation */
        VulkanSwapChain m_swapChain;

        /** Pipeline used to describe rendering process */
        VulkanPipeline m_defaultPipeline;

        VkCommandPool m_graphicsCommandPool;

        VkBuffer m_dummyVertexBuffer;
        VkDeviceMemory m_dummyVertexBufferMem;

        uint64_t m_dummyIndexBufferIndexCount;
        VkBuffer m_dummyIndexBuffer;
        VkDeviceMemory m_dummyIndexBufferMem;

        VkImage m_dummyTextureImage;
        VkDeviceMemory m_dummyTextureImageMemory;
        VkImageView m_dummyTextureImageView;
        VkSampler m_dummyTextureSampler;

        VkDescriptorSetLayout m_globalUBODescriptorSetLayout;
        VkDescriptorPool m_globalUBODescriptorPool;

        VkDescriptorPool m_imguiDescriptorPool;

        /**
         * Data representing a render frames in flight
         */
        struct RenderFrameData
        {
            std::array<VkCommandBuffer, kMaxFramesInFlight> commandBuffers;

            std::array<VkSemaphore, kMaxFramesInFlight> imageAvailableSemaphores;
            std::array<VkSemaphore, kMaxFramesInFlight> renderFinishedSemaphores;
            std::array<VkFence, kMaxFramesInFlight> inFlightFences;

            std::array<VkBuffer, kMaxFramesInFlight> globalUniformBuffers;
            std::array<VkDeviceMemory, kMaxFramesInFlight> globalUniformBuffersMemory;
            std::array<void*, kMaxFramesInFlight> globalUniformBuffersMapped;
            std::array<VkDescriptorSet, kMaxFramesInFlight> globalUBODescriptorSets;

            void Init(VkDevice device, VkCommandPool commandPool);
            void DeInit(VkDevice device);
        } m_frameData;

        uint32_t m_frameCount = 0;
    };

} // namespace sj
