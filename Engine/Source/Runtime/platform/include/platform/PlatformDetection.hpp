#pragma once
// STD Headers

// Library Headers

// Screwjank Headers

/**
 * Detect operating systems
 */
#ifdef _WIN64
    #define SJ_PLATFORM_WINDOWS
#elif __APPLE__
    #define SJ_PLATFORM_IOS
#elif __linux__
    #define SJ_PLATFORM_LINUX
#endif

/**
 *  Detect compiler
 */
#ifdef _MSC_VER
    #define SJ_MSVC
#elif __GNUC__
    #define SJ_GCC
#elif __clang__
    #define SJ_CLANG
#else
    #error "Compiler not supported."
#endif // _MSC_VER

/**
 * Detect build configuration
 */
#ifdef NDEBUG
    #define SJ_RELEASE
#else
    #define SJ_DEBUG
    #define SJ_ENABLE_ASSERTS
#endif // NDEBUG
