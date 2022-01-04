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
    ThreadPool& ThreadPool::Get()
    {
        static ThreadPool s_pool;

        return s_pool;
    }

    ThreadPool::ThreadPool()
        : m_ThreadPoolWorkBuffer(MemorySystem::GetRootHeapZone(), sizeof(ThreadJob) * 128, "Thread pool work buffer")
    {
        // Main thread + n worker threads
        int worker_count = std::thread::hardware_concurrency() - 1;
        
        SJ_ENGINE_LOG_INFO("Spawning {} worker threads", worker_count);

        for (int i = 0; i < worker_count; i++)
        {
            int worker_id = i + 1;
            auto worker_function = [this, worker_id]() {
                while (!m_Terminated)
                {
                    SJ_ENGINE_LOG_INFO("Worker thread {} running on core {}",
                                       worker_id,
                                       GetCurrentProcessorNumber());

                    std::this_thread::sleep_for(std::chrono::seconds(20));
                }
            };

            m_Threads.Add(Thread(i+1, worker_function));
        }

    }
    
    ThreadPool::~ThreadPool()
    {
        m_Terminated = true;
        for (Thread& thread : m_Threads)
        {
            thread.Join();
        }
    }
} // namespace sj