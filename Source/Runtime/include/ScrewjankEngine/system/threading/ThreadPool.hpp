#pragma once

// STD Headers
#include <thread>

// Screwjank Headers
#include <ScrewjankEngine/containers/Vector.hpp>
#include <ScrewjankEngine/system/threading/Thread.hpp>

namespace sj
{
    class ThreadPool
    {
      public:
        static ThreadPool& Get();

      private:
        ThreadPool();
        ~ThreadPool();

        StaticVector<Thread, 32> m_Threads;
    };
}