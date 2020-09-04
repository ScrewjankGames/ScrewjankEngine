#pragma once
// STD Headers
#include <cstdio>
#include <cassert>

// Library Headers
#include <spdlog/spdlog.h>

// Screwjank Headers
#include "core/Name.hpp"

namespace Screwjank {

    class Logger
    {
      public:
        Logger(const char* name);
        ~Logger() = default;

        static Logger* GetEngineLogger();
        static Logger* GetGameLogger();

        template <typename... Args>
        void LogTrace(const char* format, Args... args);

        template <typename... Args>
        void LogInfo(const char* format, Args... args);

      private:
        const char* m_Name;
    };

    template <typename... Args>
    inline void Logger::LogTrace(const char* format, Args... args)
    {
        spdlog::trace(format, args...)
    }

    template <typename... Args>
    inline void Logger::LogInfo(const char* format, Args... args)
    {
        spdlog::info(format, args...);
    }
} // namespace Screwjank

#ifdef NDEBUG
#define SJ_ENGINE_LOG_TRACE(...)
#define SJ_ENGINE_LOG_INFO(format, ...) Screwjank::Logger::GetEngineLogger()
#define SJ_ENGINE_LOG_WARN(...)
#define SJ_ENGINE_LOG_ERROR(...)

#define SJ_LOG_TRACE(...)
#define SJ_LOG_INFO(...)
#define SJ_LOG_WARN(...)
#define SJ_LOG_ERROR(...)
#else
#define SJ_ENGINE_LOG_TRACE(format, ...)                                                           \
    Screwjank::Logger::GetEngineLogger()->LogInfo(format, __VA_ARGS__)
#define SJ_ENGINE_LOG_INFO(format, ...)                                                            \
    Screwjank::Logger::GetEngineLogger()->LogInfo(format, __VA_ARGS__);
#define SJ_ENGINE_LOG_WARN(...) // Screwjank::Logger::GetEngineLogger().warn(__VA_ARGS__)
#define SJ_ENGINE_LOG_ERROR(...) // Screwjank::Logger::GetEngineLogger().error(__VA_ARGS__)

#define SJ_LOG_TRACE(...)
#define SJ_LOG_INFO(...)
#define SJ_LOG_WARN(...)
#define SJ_LOG_ERROR(...)

#endif
