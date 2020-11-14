#pragma once

// STD Headers

// Library Headers

// Screwjank Headers
#include "system/Memory.hpp"

// Platform Specific Headers

namespace sj {

    class RendererAPI
    {
      public:
        /**
         * Creates and initializes graphics API
         */
        static UniquePtr<RendererAPI> Create();

        /**
         * Destructor
         */
        virtual ~RendererAPI() = default;

      protected:
        /**
         * Constructor
         */
        RendererAPI() = default;

      private:
    };

} // namespace sj
