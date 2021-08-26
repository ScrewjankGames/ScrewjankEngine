#pragma once

// STD Headers

// Library Headers


// Screwjank Headers
#include <containers/Vector.hpp>
#include <system/Memory.hpp>

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
