#pragma once

#include "Macro/Configs.hpp"
#include "Logger/Logger.hpp"


#if WORSE_NO_LOGGING

    #define LOG_TRACE(target, fmt, ...)
    #define LOG_DEBUG(target, fmt, ...)
    #define LOG_INFO(target, fmt, ...)
    #define LOG_WARN(target, fmt, ...)
    #define LOG_ERROR(target, fmt, ...)
    #define LOG_FATAL(target, fmt, ...)

#else

    #define LOG_TRACE(target, fmt, ...)                                                                         \
        do                                                                                                      \
        {                                                                                                       \
            ::worse::GetLoggerInstance()->Log(::worse::ELogLevel::Trace, target, fmt __VA_OPT__(, ) __VA_ARGS__); \
        } while (false)
    #define LOG_DEBUG(target, fmt, ...)                                                                         \
        do                                                                                                      \
        {                                                                                                       \
            ::worse::GetLoggerInstance()->Log(::worse::ELogLevel::Debug, target, fmt __VA_OPT__(, ) __VA_ARGS__); \
        } while (false)
    #define LOG_INFO(target, fmt, ...)                                                                         \
        do                                                                                                     \
        {                                                                                                      \
            ::worse::GetLoggerInstance()->Log(::worse::ELogLevel::Info, target, fmt __VA_OPT__(, ) __VA_ARGS__); \
        } while (false)
    #define LOG_WARN(target, fmt, ...)                                                                         \
        do                                                                                                     \
        {                                                                                                      \
            ::worse::GetLoggerInstance()->Log(::worse::ELogLevel::Warn, target, fmt __VA_OPT__(, ) __VA_ARGS__); \
        } while (false)
    #define LOG_ERROR(target, fmt, ...)                                                                         \
        do                                                                                                      \
        {                                                                                                       \
            ::worse::GetLoggerInstance()->Log(::worse::ELogLevel::Error, target, fmt __VA_OPT__(, ) __VA_ARGS__); \
        } while (false)
    #define LOG_FATAL(target, fmt, ...)                                                                         \
        do                                                                                                      \
        {                                                                                                       \
            ::worse::GetLoggerInstance()->Log(::worse::ELogLevel::Fatal, target, fmt __VA_OPT__(, ) __VA_ARGS__); \
        } while (false)

#endif
