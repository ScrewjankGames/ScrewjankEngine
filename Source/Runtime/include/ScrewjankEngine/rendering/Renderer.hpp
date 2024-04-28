#pragma once

// STD Headers

// Screwjank Headers
#include <ScrewjankEngine/system/memory/Memory.hpp>

namespace sj {

    // Forward declarations
    class VulkanRendererAPI;

    class Renderer
    {
    public:
        static MemSpace<FreeListAllocator>* WorkBuffer();

        /**
         * Constructor
         */
        Renderer() = default;

        /**
         * Destructor
         */
        ~Renderer() = default;

        void Init();
        void DeInit();

        void StartRenderFrame();

        /**
         * Execute draw commands 
         */
        void Render();

      private:
        VulkanRendererAPI* m_renderAPI;
    };

} // namespace sj
