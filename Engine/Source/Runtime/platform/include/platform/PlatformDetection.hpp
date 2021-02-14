#pragma once
// STD Headers

// Library Headers

// Screwjank Headers

/**
 * Detect operating systems
 */

namespace sj {

    /**
     * Detect operating system
     */
    enum class Platform
    {
        Windows,
        Linux,
        IOS,
        Unknown
    };

#ifdef _WIN64
    #define SJ_PLATFORM_WINDOWS
    constexpr Platform g_Platform = Platform::Windows;
#elif __linux__
    #define SJ_PLATFORM_LINUX
    constexpr Platform g_Platform = Platform::Linux;
#elif __APPLE__
    #define SJ_PLATFORM_IOS
    constexpr Platform g_Platform = Platform::IOS;
#else
    #define SJ_PLATFORM_UNKNOWN
    constexpr Platform g_Platform = Platform::Unknown;
#endif

    /**
     *  Detect compiler toolchain
     */
    enum class Compiler
    {
        MSVC,
        Clang,
        GCC,
        Unknown
    };
#ifdef _MSC_VER
    #define SJ_COMPILER_MSVC
    constexpr Compiler g_Compiler = Compiler::MSVC;
#elif __GNUC__
    #define SJ_COMPILER_GCC
    constexpr Compiler g_Compiler = Compiler::GCC;
#elif __clang__
    #define SJ_COMPILER_CLANG
    constexpr Compiler g_Compiler = Compiler::Clang;
#else
    #define SJ_COMPILER_UNKNOWN
    constexpr Compiler g_Compiler = Compiler::Unknown;
#endif // _MSC_VER

    /**
     * Detect build configuration
     */
#ifdef NDEBUG
    #define SJ_RELEASE
    constexpr bool g_IsDebugBuild = false;
#else
    #define SJ_DEBUG
    #define SJ_ENABLE_ASSERTS
    constexpr bool g_IsDebugBuild = true;
#endif // NDEBUG

    /**
     * Detect rendering API support  
     */
#ifdef SJ_VULKAN_SUPPORT
    constexpr bool g_VulkanEnabled = true;
#else
    constexpr bool g_VulkanEnabled = false;
#endif 

#ifdef SJ_DX12_SUPPORT
    constexpr bool g_DirectX12Enabled = true;
 #else
    constexpr bool g_DirectX12Enabled = false;
#endif 


} // namespace sj
