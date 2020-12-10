#pragma once

// STD Headers

// Library Headers

// Screwjank Headers
#include "system/Memory.hpp"

namespace sj {
    class RenderDevice
    {
      public:
        /**
         * Creates a render device based on the current rendering API
         */
        static UniquePtr<RenderDevice> Create();

      protected:
        /**
         * Constructor
         */
        RenderDevice() = default;
    };
} // namespace sj
