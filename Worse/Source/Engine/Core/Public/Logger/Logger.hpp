#pragma once

#include "BaseTypes.hpp"

#include <format>
#include <string_view>
#include <filesystem>

namespace worse
{

    enum class ELogLevel
    {
        Trace,
        Debug,
        Info,
        Warn,
        Error,
        Max,
    };

    struct LoggerDesc
    {
        Size maxQueueSize             = 4096;
        Size maxLogFileSize           = 5 * 1024 * 1024;
        Size maxFileCount             = 3;
        std::filesystem::path logPath = "worse-engine.log";
    };

    class Logger
    {
    public:
        Logger(LoggerDesc const& desc);
        ~Logger();

        void LogImpl(ELogLevel level, std::string_view target, std::string_view formatted);

        template <typename... Args>
        void Log(ELogLevel level, std::string_view target, std::format_string<Args...> fmt, Args&&... args)
        {
            auto formatted = std::format(fmt, std::forward<Args>(args)...);
            LogImpl(level, target, formatted);
        }

        void Flush();
    };

    void CreateLogger(LoggerDesc const& desc);
    void DestroyLogger();
    Logger* GetLoggerInstance();

} // namespace worse