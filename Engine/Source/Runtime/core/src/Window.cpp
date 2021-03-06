// STD Headers

// Library Headers

// Screwjank Headers
#include "core/Window.hpp"

// Platform specific headers
#ifdef SJ_PLATFORM_WINDOWS
    #include "platform/Windows/WindowsWindow.hpp"
#elif SJ_PLATFORM_LINUX
//#include "platform/Linux/LinuxWindow.hpp"
    #error Linux platform unsupported
#elif SJ_PLATFORM_IOS
    #error IOS Platform unsupported
#elif
    #error Unknown Platform
#endif

namespace sj {
    UniquePtr<Window> Window::Create()
    {
        static_assert(g_Platform != Platform::Unknown,
                      "Window system does not support this platform");

        if constexpr (g_Platform == Platform::Windows) {
            WindowsWindow* window = New<WindowsWindow>();
            return UniquePtr<Window>(window, [](auto* ptr) {
                Delete<Window>(ptr);
            });
        } else if constexpr (g_Platform == Platform::Linux) {
            return nullptr;
        } else if constexpr (g_Platform == Platform::IOS) {
            return nullptr;
        }
    }
} // namespace sj
