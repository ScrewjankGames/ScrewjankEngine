// STD Headers

// Library Headers

// Screwjank Headers
#include <ScrewjankEngine/rendering/RendererAPI.hpp>
#include <ScrewjankEngine/rendering/Renderer.hpp>
#include <ScrewjankEngine/rendering/RenderDevice.hpp>
#include <ScrewjankEngine/platform/PlatformDetection.hpp>

// Platform specific headers
#ifdef SJ_PLATFORM_WINDOWS
    #include <ScrewjankEngine/platform/Vulkan/VulkanRendererAPI.hpp>
#elif SJ_PLATFORM_LINUX
    #error Linux platform unsupported
#elif SJ_PLATFORM_IOS
    #error IOS Platform unsupported
#elif
    #error Unknown Platform
#endif

namespace sj {

    RendererAPI* RendererAPI::Create()
    {
        static_assert(g_Platform != Platform::Unknown, "Renderer does not support this platform");

        RendererAPI* api = nullptr;

        // Chose the rendering API
        if constexpr (g_Platform == Platform::Windows) 
        {
            api = VulkanRendererAPI::GetInstance();
        } 
        else if constexpr (g_Platform == Platform::Linux) 
        {
            SJ_ASSERT_NOT_IMPLEMENTED();
        } 
        else if constexpr (g_Platform == Platform::IOS) 
        {
            SJ_ASSERT_NOT_IMPLEMENTED();
        }

        SJ_ASSERT(api != nullptr, "Failed to initialize rendering API");

        api->Init();
        return api;
    }
} // namespace sj
