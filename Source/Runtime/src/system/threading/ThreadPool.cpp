// Parent Header
#include <ScrewjankEngine/system/threading/ThreadPool.hpp>

// STD Headers
#include <mutex>

// Engine Headers
#include <ScrewjankEngine/system/threading/Thread.hpp>

// TODO: Remove me
#ifdef SJ_PLATFORM_WINDOWS
#include <Windows.h>
#endif // SJ_PLATFORM_WINDOWS

namespace sj
{
    // TODO [NL]: Remove me
    std::mutex g_TestIoMutex;

    ThreadPool& ThreadPool::Get()
    {
        static ThreadPool s_pool;

        return s_pool;
    }

    ThreadPool::ThreadPool()
    {
        // Main thread + n worker threads
        int worker_count = std::thread::hardware_concurrency() - 1;
        
        SJ_ENGINE_LOG_INFO("Spawning {} worker threads", worker_count);

        for (int i = 0; i < worker_count; i++)
        {
            int worker_id = i + 1;
            auto worker_function = [worker_id]() {
                while (true)
                {
                    //std::lock_guard<std::mutex> ioGuard(g_TestIoMutex);

                    SJ_ENGINE_LOG_INFO("Worker thread {} running on core {}",
                                       worker_id,
                                       GetCurrentProcessorNumber());

                    std::this_thread::sleep_for(std::chrono::seconds(5));
                }
            };

            m_Threads.Add(Thread(i+1, worker_function));
        }

    }
    
    ThreadPool::~ThreadPool()
    {
        for (Thread& thread : m_Threads)
        {
            thread.Join();
        }
    }
} // namespace sj