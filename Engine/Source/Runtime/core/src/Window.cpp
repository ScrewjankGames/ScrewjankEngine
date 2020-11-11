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
    UniquePtr<Window> Window::MakeWindow()
    {
#ifdef SJ_PLATFORM_WINDOWS
        WindowsWindow* window = New<WindowsWindow>();
        return UniquePtr<Window>(window, [](auto* p) {
            MemorySystem::GetDefaultAllocator()->Free(p);
        });
#else
        return nullptr;
#endif
    }
} // namespace sj
