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

        VulkanSwapChain() = default;
        ~VulkanSwapChain();

        void Init(Window* targetWindow,
                  VkPhysicalDevice physicalDevice,
                  VkDevice logicalDevice,
                  VkSurfaceKHR renderingSurface);

        void DeInit();

        VkExtent2D GetExtent() const;
        
        VkFormat GetImageFormat() const;

        VkSwapchainKHR GetSwapChain() const;

        /**
         * Query swap chain support parameters
         */
        static SwapChainParams QuerySwapChainParams(VkPhysicalDevice physical_device, 
                                                    VkSurfaceKHR surface);

        std::span<VkImageView> GetImageViews() const;

      private:
        /**
         * Handles initial swap chain setup
         */
        void InitializeSwapChain(VkSurfaceKHR renderingSurface,
                                 VkPhysicalDevice physicalDevice, 
                                 const SwapChainParams& params);

        /**
         * Communicates with the window to query swap chain extents
         */
        VkExtent2D QuerySwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        
        bool m_IsInitialized = false;

        /** Non-owning handle to the window the swap chain presents to */
        Window* m_TargetWindow = nullptr;

        VkDevice m_LogicalDevice = VK_NULL_HANDLE;

        /** Pointer to the swap chain that controls image presenation */
        VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;

        /** List of handles to images in the swap chain */
        dynamic_vector<VkImage> m_Images;

        /** List of Image Views onto the images in the swap chain */
        dynamic_vector<VkImageView> m_ImageViews;

        /** Format for images in the swap chain */
        VkFormat m_ChainImageFormat = VK_FORMAT_UNDEFINED;

        /** Size of the images in the swap chain (in pixels) */
        VkExtent2D m_ImageExtent;

    };

}