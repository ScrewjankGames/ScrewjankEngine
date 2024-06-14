#pragma once
// Screwjank Headers
#include <ScrewjankShared/utils/PlatformDetection.hpp>

#ifdef SJ_PLATFORM_LINUX
    #include <signal.h>
#endif

#ifdef SJ_DEBUG
    #ifdef SJ_PLATFORM_WINDOWS
        #define SJ_DEBUGBREAK() __debugbreak()
    #elif SJ_PLATFORM_LINUX
        #define SJ_DEBUGBREAK() raise(SIGTRAP)
    #else
        #error "Debug break NYI for this platform"
    #endif // SJ_PLATFORM_WINDOWS
#else
    #define SJ_DEBUGBREAK()
#endif // SJ_DEBUG

#ifdef SJ_ENABLE_ASSERTS
    #define SJ_ASSERT(condition, fmt, ...)                                                         \
        if (!(condition)) {                                                                        \
            SJ_DEBUGBREAK();                                                                       \
        }

    #define SJ_ASSERT_NOT_IMPLEMENTED(...)                                                         \
        SJ_DEBUGBREAK();
#else
    #define SJ_ASSERT(...)
    #define SJ_ASSERT_NOT_IMPLEMENTED(...)
#endif // SJ_ENABLE_ASSERTS
