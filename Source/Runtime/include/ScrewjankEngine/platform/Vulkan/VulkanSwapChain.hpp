#pragma once

// STD Headers
#include <span>

// Library Headers
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

// Screwjank Headers

import sj.std.containers;

namespace sj {

    class Renderer;
    class Window;
    
    class VulkanSwapChain
    {
      public:
        /**
         * Structure binding the parameters needed to build a swap chain
         */
        struct SwapChainParams
        {
            VkSurfaceCapabilitiesKHR Capabilities;
            dynamic_array<VkSurfaceFormatKHR> Formats;
            dynamic_array<VkPresentModeKHR> PresentModes;
        };

        VulkanSwapChain();
        ~VulkanSwapChain() = default;

        void Init(VkPhysicalDevice physicalDevice,
                  VkDevice logicalDevice,
                  VkSurfaceKHR renderingSurface);

        /**
         * Called after init because info from the swap chain is need to create the render pass 
         */
        void InitFrameBuffers(VkDevice device, VkRenderPass pass);

        void DeInit(VkDevice logicalDevice);

        void Recreate(VkPhysicalDevice physicalDevice, 
                      VkDevice device, 
                      VkSurfaceKHR renderingSurface,
                      VkRenderPass pass);

        [[nodiscard]] VkExtent2D GetExtent() const;
        
        [[nodiscard]] VkFormat GetImageFormat() const;

        [[nodiscard]] VkSwapchainKHR GetSwapChain() const;

        /**
         * Query swap chain support parameters
         */
        static SwapChainParams QuerySwapChainParams(VkPhysicalDevice physical_device, 
                                                    VkSurfaceKHR surface);

        [[nodiscard]] std::span<VkImageView> GetImageViews() const;

        [[nodiscard]] std::span<VkFramebuffer> GetFrameBuffers() const;

        [[nodiscard]] VkSemaphore GetImageRenderCompleteSemaphore(uint32_t imageIdx);

      private:
        /**
         * Communicates with the window to query swap chain extents
         */
        VkExtent2D QuerySwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        
        void CreateDepthResources(VkDevice logicalDevice, VkPhysicalDevice physicalDevice);

        bool m_isInitialized = false;

        /** Non-owning handle to the window the swap chain presents to */
        Window* m_targetWindow = nullptr;

        /** Pointer to the swap chain that controls image presenation */
        VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;

        /** Semaphors to track when each image is presented  */
        dynamic_array<VkSemaphore> m_renderFinishedSemaphores;

        /** List of handles to images in the swap chain */
        dynamic_vector<VkImage> m_images;

        /** List of Image Views onto the images in the swap chain */
        dynamic_vector<VkImageView> m_imageViews;

        /** Frame buffers for images in the swap chain */
        dynamic_vector<VkFramebuffer> m_swapChainBuffers;

        /** Format for images in the swap chain */
        VkFormat m_chainImageFormat = VK_FORMAT_UNDEFINED;

        /** Size of the images in the swap chain (in pixels) */
        VkExtent2D m_imageExtent = {};

        /** Depth buffer resources */
        VkImage m_depthImage{};
        VkDeviceMemory m_depthImageMemory{};
        VkImageView m_depthImageView{};
    };

}