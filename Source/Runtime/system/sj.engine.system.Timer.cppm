module;

#include <chrono>

export module sj.engine.system.Timer;

export namespace sj
{
    class Timer
    {
    public:
        using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

        Timer() : m_start(std::chrono::steady_clock::now())
        {
        }

        void Reset()
        {
            m_start = std::chrono::steady_clock::now();
        }

        [[nodiscard]] TimePoint Now() const
        {
            return std::chrono::steady_clock::now();
        }

        [[nodiscard]] float Elapsed() const
        {
            TimePoint now = std::chrono::steady_clock::now();
            return std::chrono::duration<float>(now - m_start).count();
        }

    private:
        TimePoint m_start;
    };
} // namespace sj