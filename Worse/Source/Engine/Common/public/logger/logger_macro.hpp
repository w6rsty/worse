#pragma once
#include "macro/build.hpp"
#include "logger/logger.hpp"

#if NO_LOGGING

    #define WORSE_LOG_TRACE(target, fmt, ...)
    #define WORSE_LOG_DEBUG(target, fmt, ...)
    #define WORSE_LOG_INFO(target, fmt, ...)
    #define WORSE_LOG_WARN(target, fmt, ...)
    #define WORSE_LOG_ERROR(target, fmt, ...)
    #define WORSE_LOG_FATAL(target, fmt, ...)

#else

    #define WORSE_LOG_TRACE(target, fmt, ...)                                                                  \
        do                                                                                                     \
        {                                                                                                      \
            ::Worse::Logger::Instance().Log(::Worse::LogLevel::Trace, target, fmt __VA_OPT__(, ) __VA_ARGS__); \
        } while (false)
    #define WORSE_LOG_DEBUG(target, fmt, ...)                                                                  \
        do                                                                                                     \
        {                                                                                                      \
            ::Worse::Logger::Instance().Log(::Worse::LogLevel::Debug, target, fmt __VA_OPT__(, ) __VA_ARGS__); \
        } while (false)
    #define WORSE_LOG_INFO(target, fmt, ...)                                                                  \
        do                                                                                                    \
        {                                                                                                     \
            ::Worse::Logger::Instance().Log(::Worse::LogLevel::Info, target, fmt __VA_OPT__(, ) __VA_ARGS__); \
        } while (false)
    #define WORSE_LOG_WARN(target, fmt, ...)                                                                  \
        do                                                                                                    \
        {                                                                                                     \
            ::Worse::Logger::Instance().Log(::Worse::LogLevel::Warn, target, fmt __VA_OPT__(, ) __VA_ARGS__); \
        } while (false)
    #define WORSE_LOG_ERROR(target, fmt, ...)                                                                  \
        do                                                                                                     \
        {                                                                                                      \
            ::Worse::Logger::Instance().Log(::Worse::LogLevel::Error, target, fmt __VA_OPT__(, ) __VA_ARGS__); \
        } while (false)
    #define WORSE_LOG_FATAL(target, fmt, ...)                                                                  \
        do                                                                                                     \
        {                                                                                                      \
            ::Worse::Logger::Instance().Log(::Worse::LogLevel::Fatal, target, fmt __VA_OPT__(, ) __VA_ARGS__); \
        } while (false)

#endif
