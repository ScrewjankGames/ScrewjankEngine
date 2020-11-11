#pragma once

// STD Headers

// Library Headers

// Screwjank Headers
#include "core/Window.hpp"

struct GLFWwindow;

namespace sj {

    /**
     * @brief Platform-specific implementation of a windows window
     */
    class WindowsWindow : public Window
    {
      public:
        /**
         * Constructor
         */
        WindowsWindow();

        /**
         * Destructor
         */
        ~WindowsWindow();

        /**
         * Pump the window's event queue
         */
        void ProcessEvents() override;

        /**
         * @return true If the window has been instructed to close, else false
         */
        bool WindowClosed() const override;

      private:
        GLFWwindow* m_NativeWindow;
    };

} // namespace sj
