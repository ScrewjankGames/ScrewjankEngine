#pragma once

// Parent Include
#include <ScrewjankEngine/system/threading/Thread.hpp>

// STD Includes

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
        // TODO: Thread Affinity?
    }
}