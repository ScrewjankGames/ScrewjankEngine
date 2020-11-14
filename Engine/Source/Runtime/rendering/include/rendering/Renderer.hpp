#pragma once

// STD Headers

// Library Headers
#include <vulkan/vulkan.h>

// Screwjank Headers
#include "containers/Vector.hpp"
#include "system/Memory.hpp"

namespace sj {

    class RendererAPI;

    class Renderer
    {
      public:
        /**
         * Constructor
         */
        Renderer();

        /**
         * Destructor
         */
        ~Renderer();

      private:
        UniquePtr<RendererAPI> m_RendererAPI;
    };

} // namespace sj
