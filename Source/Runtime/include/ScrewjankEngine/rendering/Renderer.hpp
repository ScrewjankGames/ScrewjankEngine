#pragma once

// STD Headers

// Library Headers


// Screwjank Headers
#include <ScrewjankEngine/containers/Vector.hpp>
#include <ScrewjankEngine/system/memory/Memory.hpp>

namespace sj {

    class RendererAPI;
    class Window;

    class Renderer
    {
    public:
        static MemSpace<FreeListAllocator>* WorkBuffer();

        /**
         * Constructor
         */
        Renderer();

        /**
         * Destructor
         */
        ~Renderer();

        /**
         * Execute draw commands 
         */
        void Render();

      private:
        RendererAPI* m_RendererAPI = nullptr;
    };

} // namespace sj
