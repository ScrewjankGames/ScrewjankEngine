#pragma once

// STD Headers

// Library Headers

// Screwjank Headers
#include "system/Memory.hpp"

namespace sj {

    class Window
    {
      public:
        /**
         * Factory function to construct Window interface appropriate to the current operating
         * system
         */
        static UniquePtr<Window> MakeWindow();

        /**
         * Destructor
         */
        ~Window() = default;

        /**
         * Pump the window's event queue
         */
        virtual void ProcessEvents() = 0;

        /**
         * @return true If the window has been instructed to close, else false
         */
        virtual bool WindowClosed() const = 0;

      protected:
        /**
         * Constructor
         */
        Window() = default;
    };

} // namespace sj
