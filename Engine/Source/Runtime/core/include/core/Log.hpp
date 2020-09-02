#pragma once
// STD Headers
#include <cstdio>
#include <cassert>

// Library Headers

// Screwjank Headers

namespace Screwjank {
    class Logger
    {
    };
} // namespace Screwjank

#ifdef NDEBUG
#define SJ_ENGINE_LOG_TRACE(...)
#define SJ_ENGINE_LOG_INFO(...)
#define SJ_ENGINE_LOG_WARN(...)
#define SJ_ENGINE_LOG_ERROR(...)

#define SJ_LOG_TRACE(...)
#define SJ_LOG_INFO(...)
#define SJ_LOG_WARN(...)
#define SJ_LOG_ERROR(...)
#else
#define SJ_ENGINE_LOG_TRACE(...) // Screwjank::Logger::GetEngineLogger().trace(__VA_ARGS__)
#define SJ_ENGINE_LOG_INFO(...) // spdlog::info(__VA_ARGS__)
#define SJ_ENGINE_LOG_WARN(...) // Screwjank::Logger::GetEngineLogger().warn(__VA_ARGS__)
#define SJ_ENGINE_LOG_ERROR(...) // Screwjank::Logger::GetEngineLogger().error(__VA_ARGS__)

#define SJ_LOG_TRACE(...)
#define SJ_LOG_INFO(...)
#define SJ_LOG_WARN(...)
#define SJ_LOG_ERROR(...)

#endif
