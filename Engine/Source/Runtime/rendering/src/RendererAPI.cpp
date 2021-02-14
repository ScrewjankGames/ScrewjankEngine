// STD Headers

// Library Headers

// Screwjank Headers
#include "rendering/RendererAPI.hpp"
#include "rendering/RenderDevice.hpp"
#include "platform/PlatformDetection.hpp"

// Platform specific headers
#ifdef SJ_PLATFORM_WINDOWS
    #include "platform/Vulkan/VulkanRendererAPI.hpp"
#elif SJ_PLATFORM_LINUX
    #error Linux platform unsupported
#elif SJ_PLATFORM_IOS
    #error IOS Platform unsupported
#elif
    #error Unknown Platform
#endif

namespace sj {

    /** Defaults to Vulkan API */
    RendererAPI::API RendererAPI::s_VendorAPI = RendererAPI::API::Unkown;

    UniquePtr<RendererAPI> RendererAPI::Create(Window* window)
    {
        static_assert(g_Platform != Platform::Unknown, "Renderer does not support this platform");

        RendererAPI* api = nullptr;

        // Chose the rendering API
        if constexpr (g_Platform == Platform::Windows) 
        {
            s_VendorAPI = API::Vulkan;
            api = New<VulkanRendererAPI>(window);
        } 
        else if constexpr (g_Platform == Platform::Linux) 
        {
            SJ_ASSERT_NYI();
        } 
        else if constexpr (g_Platform == Platform::IOS) 
        {
            SJ_ASSERT_NYI();
        }

        SJ_ASSERT(api != nullptr, "Failed to initialize rendering API");

        return UniquePtr<RendererAPI>(api, [](RendererAPI* ptr) {
            Delete<RendererAPI>(ptr);
        });
    }

    RendererAPI::API RendererAPI::GetVendorAPI()
    {
        return s_VendorAPI;
    }

    RendererAPI::RendererAPI(Window* window)
    {
        m_Window = window;
    }

} // namespace sj
