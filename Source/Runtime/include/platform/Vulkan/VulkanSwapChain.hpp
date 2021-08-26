#pragma once

// STD Headers

// Library Headers
#include <vulkan/vulkan.h>

// Screwjank Headers
#include <containers/Vector.hpp>

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
            Vector<VkSurfaceFormatKHR> Formats;
            Vector<VkPresentModeKHR> PresentModes;
        };

        /**
         * Constructor 
         * @param physical_device Non-owning handle to the physical device the swap chain will be running on
         */
        VulkanSwapChain(VulkanRendererAPI* api, Window* target_window);

        /**
         * Destructor  
         */
        ~VulkanSwapChain();

        /**
         * Query swap chain support parameters
         */
        SwapChainParams QuerySwapChainParams(VkPhysicalDevice physical_device,
                                             VkSurfaceKHR surface) const;

        /**
         * Communicates with the window to query swap chain extents
         */
        VkExtent2D QuerySwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

      private:
        /** Vulkan API implementation for device and surface querying */
        VulkanRendererAPI* m_API;

        /** Non-owning handle to the window the swap chain presents to */
        Window* m_TargetWindow;

        /** Pointer to the swap chain that controls image presenation */
        VkSwapchainKHR m_SwapChain;

        /** List of handles to images in the swap chain */
        Vector<VkImage> m_Images;

        /** List of Image Views onto the images in the swap chain */
        Vector<VkImageView> m_ImageViews;

        /** Format for images in the swap chain */
        VkFormat m_ChainImageFormat;

        /** Size of the images in the swap chain (in pixels) */
        VkExtent2D m_ImageExtent;

        /**
         * Handles initial swap chain setup
         */
        void InitializeSwapChain(const SwapChainParams& params);
    };

}