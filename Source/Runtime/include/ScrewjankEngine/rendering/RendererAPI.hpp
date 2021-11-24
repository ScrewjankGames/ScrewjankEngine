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
        static UniquePtr<RendererAPI> Create();

        /**
         * Destructor
         */
        virtual ~RendererAPI() = default;

        /**
         * Allows user to get a handle to the logical device being used by the graphics API
         */
        virtual RenderDevice* GetRenderDevice() = 0;
    };

} // namespace sj
