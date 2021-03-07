#pragma once
// STD Headers
#include <cstdio>
#include <cassert>

// Library Headers
#include <spdlog/spdlog.h>

// Screwjank Headers
#include "core/Assert.hpp"

namespace sj {

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
        void LogDebug(const char* format, Args... args);

        template <typename... Args>
        void LogInfo(const char* format, Args... args);

        template <typename... Args>
        void LogWarn(const char* format, Args... args);

        template <typename... Args>
        void LogError(const char* format, Args... args);

        template <typename... Args>
        void LogFatal(const char* format, Args... args);

      private:
        const char* m_Name;
    };

    template <typename... Args>
    inline void Logger::LogTrace(const char* format, Args... args)
    {
        spdlog::trace(format, args...);
    }

    template <typename... Args>
    inline void Logger::LogDebug(const char* format, Args... args)
    {
        spdlog::debug(format, args...);
    }

    template <typename... Args>
    inline void Logger::LogInfo(const char* format, Args... args)
    {
        spdlog::info(format, args...);
    }

    template <typename... Args>
    inline void Logger::LogWarn(const char* format, Args... args)
    {
        spdlog::warn(format, args...);
    }

    template <typename... Args>
    inline void Logger::LogError(const char* format, Args... args)
    {
        spdlog::error(format, args...);
    }

    template <typename... Args>
    inline void Logger::LogFatal(const char* format, Args... args)
    {
        spdlog::critical(format, args...);
    }
} // namespace sj

#ifdef NDEBUG
    #define SJ_ENGINE_LOG_TRACE(...)
    #define SJ_ENGINE_LOG_DEBUG(...)
    #define SJ_ENGINE_LOG_INFO(...)
    #define SJ_ENGINE_LOG_WARN(...)
    #define SJ_ENGINE_LOG_ERROR(...)
    #define SJ_ENGINE_LOG_FATAL(...)

    #define SJ_GAME_LOG_TRACE(...)
    #define SJ_GAME_LOG_DEBUG(...)
    #define SJ_GAME_LOG_INFO(...)
    #define SJ_GAME_LOG_WARN(...)
    #define SJ_GAME_LOG_ERROR(...)
    #define SJ_GAME_LOG_FATAL(...)
#else
    #define SJ_ENGINE_LOG_TRACE(format, ...)                                                       \
        sj::Logger::GetEngineLogger()->LogTrace(format, __VA_ARGS__)
    #define SJ_ENGINE_LOG_DEBUG(format, ...)                                                        \
        sj::Logger::GetEngineLogger()->LogDebug(format, __VA_ARGS__)
    #define SJ_ENGINE_LOG_INFO(format, ...)                                                        \
        sj::Logger::GetEngineLogger()->LogInfo(format, __VA_ARGS__)
    #define SJ_ENGINE_LOG_WARN(format, ...)                                                        \
        sj::Logger::GetEngineLogger()->LogWarn(format, __VA_ARGS__)
    #define SJ_ENGINE_LOG_ERROR(format, ...)                                                       \
        sj::Logger::GetEngineLogger()->LogError(format, __VA_ARGS__)
    #define SJ_ENGINE_LOG_FATAL(format, ...)                                                       \
        sj::Logger::GetEngineLogger()->LogFatal(format, __VA_ARGS__)

    #define SJ_GAME_LOG_TRACE(...) SJ_ASSERT_NYI();
    #define SJ_GAME_LOG_DEBUG(...) SJ_ASSERT_NYI();
    #define SJ_GAME_LOG_INFO(...) SJ_ASSERT_NYI();
    #define SJ_GAME_LOG_WARN(...) SJ_ASSERT_NYI();
    #define SJ_GAME_LOG_ERROR(...) SJ_ASSERT_NYI();
    #define SJ_GAME_LOG_FATAL(...) SJ_ASSERT_NYI();

#endif
