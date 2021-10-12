#pragma once

#include <thread>
//https://eli.thegreenplace.net/2016/c11-threads-affinity-and-hyperthreading/
// ^^ READ ME

namespace sj
{
    class Thread
    {
      public:
        /**
        * Creates an invalid thread 
        */
        Thread() = default;

        /**
         * Creates a thread running the provided function on a thread of execution  
         * @param core Core index the thread should run on
         * @param function The function to execute
         * @parm args... Arguments to the function
         */
        template<typename Function, class... Args>
        Thread(int core, Function&& function, Args&&... args);

        /**
         * Thread handles are not copyable 
         */
        Thread(const Thread& other) = delete;

        /**
         * Move constructor 
         */
        Thread(Thread&& other) noexcept;

        ~Thread() = default;

        /**
         * Block calling thread until this thread has terminated
         */
        void Join();

      private:
        std::thread m_Thread;
    };
} // namespace sj

// Include Inlines
#include <ScrewjankEngine/system/threading/Thread.inl>