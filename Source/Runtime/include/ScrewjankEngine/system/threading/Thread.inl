// Parent Include
#include <ScrewjankEngine/system/threading/Thread.hpp>

// STD Includes
#include <type_traits>

// Shared Includes
#include <ScrewjankShared/utils/PlatformDetection.hpp>

// Conditional Includes
#ifdef SJ_PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace sj
{
    template <typename Function, class... Args>
    inline Thread::Thread(int core, Function&& function, Args&&... args) 
        : m_Thread(std::forward<Function>(function), std::forward<Args>(args)...)
    {
        // Set thread affinity in a platform specific way
#ifdef SJ_PLATFORM_WINDOWS
        SetThreadAffinityMask(m_Thread.native_handle(), static_cast<DWORD_PTR>(1) << core);
#else
        #error "Platform does not support threading"
#endif

    }
}