//// STD Headers
//
//// Library Headers
//
//// Screwjank Headers
//#include <ScrewjankEngine/core/Window.hpp>
//
//// Platform specific headers
//#ifdef SJ_PLATFORM_WINDOWS
//    #include <ScrewjankEngine/platform/Windows/WindowsWindow.hpp>
//#elif SJ_PLATFORM_LINUX
////#include <ScrewjankEngine/platform/Linux/LinuxWindow.hpp>
//    #error Linux platform unsupported
//#elif SJ_PLATFORM_IOS
//    #error IOS Platform unsupported
//#elif
//    #error Unknown Platform
//#endif
//
//namespace sj {
//   
//    Window* Window::GetInstance()
//    {
//        static_assert(g_Platform != Platform::Unknown,
//                      "Window system does not support this platform");
//
//        if constexpr(g_Platform == Platform::Windows)
//        {
//            static WindowsWindow* window = New<WindowsWindow>();
//
//            WindowsWindow* window = New<WindowsWindow>();
//            return UniquePtr<Window>(MemorySystem::GetRootHeapZone(), window);
//        }
//        else if constexpr(g_Platform == Platform::Linux)
//        {
//            return {};
//        }
//        else if constexpr(g_Platform == Platform::IOS)
//        {
//            return {};
//        }
//
//        return nullptr;
//    }
//} // namespace sj
