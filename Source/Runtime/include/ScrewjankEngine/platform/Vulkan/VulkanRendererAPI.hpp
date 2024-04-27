#pragma once

// STD Headers

// Library Headers
#include <vulkan/vulkan.h>

// Engine Headers
#include <ScrewjankEngine/containers/Array.hpp>
#include <ScrewjankEngine/containers/String.hpp>
#include <ScrewjankEngine/containers/Vector.hpp>
#include <ScrewjankEngine/platform/PlatformDetection.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanRenderDevice.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanSwapChain.hpp>
#include <ScrewjankEngine/platform/Vulkan/VulkanPipeline.hpp>

// Shared Headers
#include <ScrewjankShared/Math/Vec2.hpp>
#include <ScrewjankShared/Math/Vec3.hpp>
#include <ScrewjankShared/Math/Mat44.hpp>

namespace sj {

    // Forward declarations
    class VulkanPipeline;
    class VulkanRenderDevice;
    class VulkanSwapChain;
    class Window;
    struct DeviceQueueFamilyIndices;

    class VulkanRendererAPI
    {
      public:
        /**
         * Constructor
         */
        VulkanRendererAPI() = default;
        
        /**
         * Destructor
         */
        ~VulkanRendererAPI();

        void Init();
        void DeInit();

        static VulkanRendererAPI* GetInstance();

        /**
         * @return The active VkInstance
         */
        VkInstance GetVkInstanceHandle() const;

        void Draw();

        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        struct DummyVertex
        {
            Vec2 pos;
            Vec3 color;

            static VkVertexInputBindingDescription GetBindingDescription()
            {
                VkVertexInputBindingDescription desc {};

                desc.binding = 0;
                desc.stride = sizeof(DummyVertex);
                desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

                return desc;
            }

            static array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
            {
                std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions {};

                attributeDescriptions[0].binding = 0;
                attributeDescriptions[0].location = 0;
                attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
                attributeDescriptions[0].offset = offsetof(DummyVertex, pos);

                attributeDescriptions[1].binding = 0;
                attributeDescriptions[1].location = 1;
                attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[1].offset = offsetof(DummyVertex, color);

                return attributeDescriptions;
            }
        };

        /**
         * Global state shared by all shaders 
         */
        struct GlobalUniformBufferObject
        {
            Mat44 model;
            Mat44 view;
            Mat44 projection;
        };

        array<DummyVertex, 4> m_dummyVertices = {
            DummyVertex {Vec2 {-0.5f, -0.5f}, Vec3 {1.0f, 0.0f, 0.0f}},
            DummyVertex {Vec2 {0.5f, -0.5f}, Vec3 {0.0f, 1.0f, 0.0f}},
            DummyVertex {Vec2 {0.5f, 0.5f}, Vec3 {0.0f, 0.0f, 1.0f}}, 
            DummyVertex {Vec2 {-0.5f, 0.5f}, Vec3 {1.0f, 1.0f, 1.0f}}
        };

        array<uint16_t, 6> m_dummyIndices = {0, 1, 2, 2, 3, 0};

      private:

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

        /**
         * Initializes the Vulkan API's instance and debug messaging hooks
         */
        void InitializeVulkan();

        /**
         * Returns list of Vulkan extenstions the renderer should support
         */
        dynamic_vector<const char*> GetRequiredExtenstions() const;

        /** 
         * Communicates with the Window to creates the rendering surface 
         */
        void CreateRenderSurface();

        /**
         * Creates default render pass 
         */
        void CreateRenderPass();

        /**
         * Turns on Vulkan validation layers
         */
        void EnableValidationLayers(const dynamic_vector<const char*>& required_validation_layers);

        /**
         * Enables Vulkan Debug messaging
         */
        void EnableDebugMessaging();

        void CreateCommandPools();

        void CreateBuffer(VkDeviceSize size,
                          VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties,
                          VkBuffer& out_buffer,
                          VkDeviceMemory& out_bufferMemory);

        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        void CreateDummyVertexBuffer();

        void CreateDummyIndexBuffer();

        void CreateGlobalDescriptorSetlayout();
        void CreateGlobalUniformBuffers();

        void CreateGlobalUBODescriptorPool();

        void CreateGlobalUBODescriptorSets();

        void RecordCommandBuffer(VkCommandBuffer buffer, uint32_t frameIdx, uint32_t imageIdx);

        void UpdateUniformBuffer(void* bufferMem);

        bool m_IsInitialized = false;

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
        VkCommandPool m_transferCommandPool;


        VkBuffer m_dummyVertexBuffer;
        VkDeviceMemory m_dummyVertexBufferMem;

        VkBuffer m_dummyIndexBuffer;
        VkDeviceMemory m_dummyIndexBufferMem;

        VkDescriptorSetLayout m_globalUBODescriptorSetLayout;
        VkDescriptorPool m_globalUBODescriptorPool;

        /**
         * Data representing a render frames in flight 
         */
        struct RenderFrameData
        {
            array<VkCommandBuffer, kMaxFramesInFlight> commandBuffers;
            
            array<VkSemaphore, kMaxFramesInFlight> imageAvailableSemaphores;
            array<VkSemaphore, kMaxFramesInFlight> renderFinishedSemaphores;
            array<VkFence, kMaxFramesInFlight> inFlightFences;

            array<VkBuffer, kMaxFramesInFlight> globalUniformBuffers;
            array<VkDeviceMemory, kMaxFramesInFlight> globalUniformBuffersMemory;
            array<void*, kMaxFramesInFlight> globalUniformBuffersMapped;
            array<VkDescriptorSet, kMaxFramesInFlight> globalUBODescriptorSets;


            void Init(VkDevice device, VkCommandPool commandPool);
            void DeInit(VkDevice device);
        } m_frameData;

        struct ImmediateModeData
        {
            VkFence fence;
            VkCommandBuffer commandBuffer;
            VkCommandPool commandPool;
        } m_immediateModeData;

        uint32_t m_frameCount = 0;
    };

} // namespace sj
