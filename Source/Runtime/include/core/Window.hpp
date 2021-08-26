#pragma once

// STD Headers

// Library Headers

// Screwjank Headers
#include <system/Memory.hpp>

namespace sj {

    class Window
    {
      public:
        /**
         * Struct to represent the window width and height in pixels
         */
        struct FrameBufferSize
        {
            uint32_t Width;
            uint32_t Height;
        };

        /**
         * Factory function to construct Window interface appropriate to the current operating
         * system
         */
        static UniquePtr<Window> Create();

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

        /**
         * @return Window size in pixels  
         */
        virtual FrameBufferSize GetFrameBufferSize() const = 0;

      protected:
        /**
         * Constructor
         */
        Window() = default;
    };

} // namespace sj
