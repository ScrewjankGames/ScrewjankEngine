#pragma once

// STD Headers

// Library Headers

// Screwjank Headers
#include <ScrewjankEngine/system/Memory.hpp>

// Platform Specific Headers

namespace sj {

    // Forward declarations
    class RenderDevice;
    class Window;

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
        static UniquePtr<RendererAPI> Create(Window* window);

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
        /** Reference to the engine's window */
        Window* m_Window;
        
        /**
         * Constructor
         */
        RendererAPI(Window* window);

      private:
        /** Allows other systems to query the currently active rendering API*/
        static API s_VendorAPI;


    };

} // namespace sj
