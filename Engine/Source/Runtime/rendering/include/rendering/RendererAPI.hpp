#pragma once

// STD Headers

// Library Headers

// Screwjank Headers
#include "system/Memory.hpp"

// Platform Specific Headers

namespace sj {

    // Forward declarations
    class RenderDevice;

    class RendererAPI
    {
      public:
        enum class API
        {
            Vulkan,
            DirectX,
            OpenGL,
            Unkown
        };

        /**
         * Creates and initializes graphics API
         */
        static UniquePtr<RendererAPI> Create();

        /**
         * @return The currently selected Graphics API
         */
        static API GetVendorAPI();

        /**
         * Destructor
         */
        virtual ~RendererAPI() = default;

        /**
         * Allows user to get a handle to the logical device being used by the graphics API
         */
        virtual RenderDevice* GetRenderDevice() = 0;

      protected:
        /**
         * Constructor
         */
        RendererAPI() = default;

      private:
        static API s_VendorAPI;
    };

} // namespace sj
