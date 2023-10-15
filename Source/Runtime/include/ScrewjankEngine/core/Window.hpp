#pragma once

// Self Include
#include <ScrewjankEngine/system/Memory/Memory.hpp>

// STD Headers

// Library Headers

// Screwjank Headers

namespace sj
{
    /**
     * Struct to represent the window width and height in pixels
     */
    struct Viewport
    {
        uint32_t Width;
        uint32_t Height;
    };
} // namespace sj


// Platform specific implementations
#ifdef SJ_PLATFORM_WINDOWS
    #include <ScrewjankEngine/platform/Windows/WindowsWindow.hpp>
#elif SJ_PLATFORM_LINUX
//#include <ScrewjankEngine/platform/Linux/LinuxWindow.hpp>
    #error Linux platform unsupported
#elif SJ_PLATFORM_IOS
    #error IOS Platform unsupported
#elif
    #error Unknown Platform
#endif


