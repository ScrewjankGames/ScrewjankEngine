#pragma once

// STD Headers
#include <cassert>

// Library Headers
#include <spdlog/spdlog.h>

#ifdef SJ_GOLD
    #define SJ_ENGINE_LOG_TRACE(...)
    #define SJ_ENGINE_LOG_DEBUG(...)
    #define SJ_ENGINE_LOG_INFO(...)
    #define SJ_ENGINE_LOG_WARN(...)
    #define SJ_ENGINE_LOG_ERROR(...)
    #define SJ_ENGINE_LOG_FATAL(...)
#else
    #define SJ_ENGINE_LOG_TRACE(format, ...)                                                       \
        spdlog::trace(format __VA_OPT__(, ) __VA_ARGS__); // NOLINT
    #define SJ_ENGINE_LOG_DEBUG(format, ...)                                                       \
        spdlog::debug(format __VA_OPT__(, ) __VA_ARGS__); // NOLINT
    #define SJ_ENGINE_LOG_INFO(format, ...)                                                        \
        spdlog::info(format __VA_OPT__(, ) __VA_ARGS__); // NOLINT
    #define SJ_ENGINE_LOG_WARN(format, ...)                                                        \
        spdlog::warn(format __VA_OPT__(, ) __VA_ARGS__); // NOLINT
    #define SJ_ENGINE_LOG_ERROR(format, ...)                                                       \
        spdlog::error(format __VA_OPT__(, ) __VA_ARGS__); // NOLINT
    #define SJ_ENGINE_LOG_FATAL(format, ...)                                                       \
        spdlog::critical(format __VA_OPT__(, ) __VA_ARGS__); // NOLINT
#endif
