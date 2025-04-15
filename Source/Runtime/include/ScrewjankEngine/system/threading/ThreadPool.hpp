#pragma once

// STD Headers
#include <thread>

// Screwjank Headers
#include <ScrewjankEngine/system/threading/Thread.hpp>
#include <ScrewjankEngine/system/memory/MemSpace.hpp>
#include <ScrewjankEngine/system/memory/allocators/PoolAllocator.hpp>

import sj.shared.containers;

namespace sj
{
    using JobFn = void (*)(void*);

    struct ThreadJob
    {
        void* arg;
        JobFn job;
    };

    class ThreadPool
    {
      public:
        static ThreadPool& Get();

      private:
        ThreadPool();
        ~ThreadPool();

        std::atomic<bool> m_Terminated = false;
        static_vector<Thread, 32> m_Threads;
    };
}