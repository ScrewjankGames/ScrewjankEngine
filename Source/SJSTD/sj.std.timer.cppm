module;

#include <chrono>

export module sj.std.timer;

export namespace sj
{
class timer
{
public:
    using time_point = std::chrono::time_point<std::chrono::steady_clock>;

    timer() : m_start(std::chrono::steady_clock::now())
    {
    }

    void reset()
    {
        m_start = std::chrono::steady_clock::now();
    }

    [[nodiscard]] time_point now() const
    {
        return std::chrono::steady_clock::now();
    }

    [[nodiscard]] float elapsed() const
    {
        time_point now = std::chrono::steady_clock::now();
        return std::chrono::duration<float>(now - m_start).count();
    }

private:
    time_point m_start;
};
} // namespace sj