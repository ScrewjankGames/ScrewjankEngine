#pragma once
// STD Headers

// Library Headers

// Screwjank Headers
#include "core/Log.hpp"
#include "platform/PlatformDetection.hpp"

#ifdef SJ_DEBUG
    #ifdef SJ_PLATFORM_WINDOWS
        #define SJ_DEBUGBREAK() __debugbreak()
    #elif SJ_PLATFORM_LINUX
        #include <signal.h>
        #define SJ_DEBUGBREAK() raise(SIGTRAP)
    #else
        #error "Platform does not support debug breaking"
    #endif // SJ_PLATFORM_WINDOWS
#else
    #define SJ_DEBUGBREAK()
#endif // SJ_DEBUG

#ifdef SJ_ENABLE_ASSERTS
    #define SJ_ASSERT(condition, fmt, ...)                                                         \
        if (!(condition)) {                                                                        \
            SJ_DEBUGBREAK();                                                                       \
        }
#else
    #define SJ_ASSERT(...)
#endif // SJ_ENABLE_ASSERTS
