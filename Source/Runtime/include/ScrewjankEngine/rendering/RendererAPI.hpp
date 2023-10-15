#pragma once

// STD Headers

// Library Headers

// Screwjank Headers
#include <ScrewjankEngine/system/memory/Memory.hpp>

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
         * Does not allocate memory.
         */
        static RendererAPI* Create();

        /**
         * Destructor
         */
        virtual ~RendererAPI() = default;

        virtual void DrawFrame() = 0;

      private:
        virtual void Init() = 0;
        virtual void DeInit() = 0;

    };

} // namespace sj
