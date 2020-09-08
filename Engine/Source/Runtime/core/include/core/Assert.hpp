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
        #define #include < signal.h>
        #define HZ_DEBUGBREAK() raise(SIGTRAP)
    #else
        #error "Platform does not support debug breaking"
    #endif // SJ_PLATFORM_WINDOWS
#else
    #define SJ_DEBUGBREAK()
#endif // SJ_DEBUG

#ifdef SJ_ENABLE_ASSERTS
    #define SJ_ASSERT(condition, ...)                                                              \
        if (!(condition)) {                                                                        \
            SJ_ENGINE_LOG_FATAL("Assertion failed: {0}", __VA_ARGS__);                             \
            SJ_DEBUGBREAK();                                                                       \
        }
#else
    #define SJ_ASSERT(...)
#endif // SJ_ENABLE_ASSERTS
