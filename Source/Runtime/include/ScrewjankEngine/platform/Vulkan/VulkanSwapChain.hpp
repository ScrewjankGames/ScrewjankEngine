#pragma once

// STD Headers
#include <span>

// Library Headers
#include <vulkan/vulkan.h>

// Screwjank Headers
#include <ScrewjankEngine/containers/Vector.hpp>

namespace sj {

    class VulkanRendererAPI;
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
            dynamic_vector<VkSurfaceFormatKHR> Formats;
            dynamic_vector<VkPresentModeKHR> PresentModes;
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

        VkExtent2D GetExtent() const;
        
        VkFormat GetImageFormat() const;

        VkSwapchainKHR GetSwapChain() const;

        /**
         * Query swap chain support parameters
         */
        static SwapChainParams QuerySwapChainParams(VkPhysicalDevice physical_device, 
                                                    VkSurfaceKHR surface);

        std::span<VkImageView> GetImageViews() const;

        std::span<VkFramebuffer> GetFrameBuffers() const;

      private:
        /**
         * Communicates with the window to query swap chain extents
         */
        VkExtent2D QuerySwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        
        bool m_isInitialized = false;

        /** Non-owning handle to the window the swap chain presents to */
        Window* m_targetWindow = nullptr;

        /** Pointer to the swap chain that controls image presenation */
        VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;

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

    };

}