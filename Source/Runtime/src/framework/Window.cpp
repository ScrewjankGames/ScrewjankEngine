//// STD Headers
//
//// Library Headers
//
//// Screwjank Headers
//#include <ScrewjankEngine/framework/Window.hpp>
//
//// Platform specific headers
//#ifdef SJ_PLATFORM_WINDOWS
//    #include <ScrewjankEngine/platform/Windows/GLFW_Window.hpp>
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
//            static GLFW_Window* window = New<GLFW_Window>();
//
//            GLFW_Window* window = New<GLFW_Window>();
//            return UniquePtr<Window>(MemorySystem::GetRootMemoryResource(), window);
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
