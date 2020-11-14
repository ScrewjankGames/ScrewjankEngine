// STD Headers

// Library Headers

// Screwjank Headers
#include "rendering/RendererAPI.hpp"
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

    UniquePtr<RendererAPI> sj::RendererAPI::Create()
    {
        static_assert(g_Platform != Platform::Unknown, "Renderer does not support this platform");

        if constexpr (g_Platform == Platform::Windows) {
            VulkanRendererAPI* vkAPI = New<VulkanRendererAPI>();
            return UniquePtr<RendererAPI>(vkAPI, [](auto* ptr) {
                Delete<RendererAPI>(ptr);
            });

            return nullptr;
        } else if constexpr (g_Platform == Platform::Linux) {
            return nullptr;
        } else if constexpr (g_Platform == Platform::IOS) {
            return nullptr;
        }
    }

} // namespace sj
