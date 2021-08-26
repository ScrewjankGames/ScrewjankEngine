#pragma once

// STD Headers

// Library Headers


// Screwjank Headers
#include <ScrewjankEngine/containers/Vector.hpp>
#include <ScrewjankEngine/system/Memory.hpp>

namespace sj {

    class RendererAPI;
    class Window;
    class Renderer
    {
      public:
        /**
         * Constructor
         */
        Renderer(Window* window);

        /**
         * Destructor
         */
        ~Renderer();

      private:
        UniquePtr<RendererAPI> m_RendererAPI;
    };

} // namespace sj
