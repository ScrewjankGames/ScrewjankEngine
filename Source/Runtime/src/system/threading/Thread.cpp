#include "..\..\..\include\ScrewjankEngine\system\threading\Thread.hpp"
namespace sj
{
    Thread::Thread(Thread&& other) noexcept 
        : m_Thread(std::move(other.m_Thread))
    {
    }

    void Thread::Join()
    {
        m_Thread.join();
    }
} // namespace sj