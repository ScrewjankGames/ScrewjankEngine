#pragma once

// STD Headers
#include <cstdint>

// Library Headers

// Screwjank Headers
#include <ScrewjankShared/utils/PlatformDetection.hpp>

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
#if defined(SJ_PLATFORM_WINDOWS) || defined (SJ_PLATFORM_LINUX)
    #include <ScrewjankEngine/platform/Windows/GLFW_Window.hpp>
#elif SJ_PLATFORM_IOS
    #error IOS Platform unsupported
#elif
    #error Unknown Platform
#endif


