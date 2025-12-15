#pragma once
#include "BaseTypes.hpp"

#include <format>
#include <string_view>

namespace worse
{

    enum class LogLevel
    {
        Trace,
        Debug,
        Info,
        Warn,
        Error,
        Max,
    };

    class Logger
    {
    public:
        static Logger& Instance();

        void Initialize(Size queueSize = kMaxQueueSize, Size maxFileSize = kMaxLogFileSize);

        void LogImpl(LogLevel level, std::string_view target, std::string_view formatted);

        template <typename... Args>
        void Log(LogLevel level, std::string_view target, std::format_string<Args...> fmt, Args&&... args)
        {
            auto formatted = std::format(fmt, std::forward<Args>(args)...);
            LogImpl(level, target, formatted);
        }

    public:
        inline static constexpr Size kMaxQueueSize   = 4096;
        inline static constexpr Size kMaxLogFileSize = 5 * 1024 * 1024;
    };

} // namespace worse