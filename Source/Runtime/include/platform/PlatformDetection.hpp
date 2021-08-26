#pragma once

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

#ifndef SJ_GOLD
    // Only allow asserts in non-gold builds
    #define SJ_ENABLE_ASSERTS
#endif // !SJ_GOLD


#ifndef NDEBUG
    #define SJ_DEBUG
    constexpr bool g_IsDebugBuild = true;
#endif

#ifdef NDEBUG
    #define SJ_RELEASE
    constexpr bool g_IsDebugBuild = false;
#endif // NDEBUG

#ifdef SJ_GOLD
    #define SJ_RELEASE
    constexpr bool g_IsDebugBuild = false;
#endif


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
