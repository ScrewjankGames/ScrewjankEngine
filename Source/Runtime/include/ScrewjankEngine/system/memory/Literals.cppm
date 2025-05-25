module;
#include <cstdint>

export module sj.engine.system.memory:Literals;

export {
    inline constexpr uint64_t operator""_KiB(unsigned long long val)
    {
        return val * 1024;
    }
    inline constexpr uint64_t operator""_MiB(unsigned long long val)
    {
        return val * 1024 * 1024;
    }
    inline constexpr uint64_t operator""_GiB(unsigned long long val)
    {
        return val * 1024 * 1024 * 1024;
    }
}
