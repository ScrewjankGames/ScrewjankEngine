#pragma once

// STD Headers
#include <cassert>
#include <print>

// Library Headers
#include <spdlog/spdlog.h>

// TODO: Fixme when logging library compiles on latest clang
#if SJ_GOLD 
    #define SJ_ENGINE_LOG_TRACE(...)
    #define SJ_ENGINE_LOG_DEBUG(...)
    #define SJ_ENGINE_LOG_INFO(...)
    #define SJ_ENGINE_LOG_WARN(...)
    #define SJ_ENGINE_LOG_ERROR(...)
    #define SJ_ENGINE_LOG_FATAL(...)
#else
    #define SJ_ENGINE_LOG_TRACE(format, ...)                                                       \
        std::println(format __VA_OPT__(, ) __VA_ARGS__);//spdlog::trace(format __VA_OPT__(, ) __VA_ARGS__); // NOLINT
    #define SJ_ENGINE_LOG_DEBUG(format, ...)                                                       \
        std::println(format __VA_OPT__(, ) __VA_ARGS__);//spdlog::debug(format __VA_OPT__(, ) __VA_ARGS__); // NOLINT
    #define SJ_ENGINE_LOG_INFO(format, ...)                                                        \
        std::println(format __VA_OPT__(, ) __VA_ARGS__);//spdlog::info(format __VA_OPT__(, ) __VA_ARGS__); // NOLINT
    #define SJ_ENGINE_LOG_WARN(format, ...)                                                        \
        std::println(format __VA_OPT__(, ) __VA_ARGS__);//spdlog::warn(format __VA_OPT__(, ) __VA_ARGS__); // NOLINT
    #define SJ_ENGINE_LOG_ERROR(format, ...)                                                       \
        std::println(format __VA_OPT__(, ) __VA_ARGS__);//spdlog::error(format __VA_OPT__(, ) __VA_ARGS__); // NOLINT
    #define SJ_ENGINE_LOG_FATAL(format, ...)                                                       \
        std::println(format __VA_OPT__(, ) __VA_ARGS__);//spdlog::critical(format __VA_OPT__(, ) __VA_ARGS__); // NOLINT
#endif
