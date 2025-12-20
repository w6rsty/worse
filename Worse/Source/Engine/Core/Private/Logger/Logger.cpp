#include "Container/StaticArray.hpp"
#include "Logger/Logger.hpp"

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include <memory>

namespace worse
{

    namespace
    {
        std::unique_ptr<Logger> s_LoggerInstance            = nullptr;
        std::shared_ptr<spdlog::async_logger> s_AsyncLogger = nullptr;

    } // namespace

    void CreateLogger(LoggerDesc const& desc)
    {
        s_LoggerInstance = std::make_unique<Logger>(desc);
    }

    void DestroyLogger()
    {
        s_LoggerInstance.reset();
    }

    worse::Logger* GetLoggerInstance()
    {
        return s_LoggerInstance ? s_LoggerInstance.get() : nullptr;
    }

    Logger::Logger(LoggerDesc const& desc)
    {
        spdlog::init_thread_pool(desc.maxQueueSize, 1);

        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto fileSink    = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(desc.logPath.string(), desc.maxLogFileSize, desc.maxFileCount);

        consoleSink->set_pattern("\x1b[90m%Y-%m-%d %T.%e\x1b[0m %^%-7l %v%$");
        fileSink->set_pattern("%Y-%m-%d %T.%e %-7l %v");

        s_AsyncLogger = std::make_shared<spdlog::async_logger>(
            "Engine",
            spdlog::sinks_init_list{consoleSink, fileSink},
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::discard_new);

        spdlog::register_logger(s_AsyncLogger);
        spdlog::set_default_logger(s_AsyncLogger);

        spdlog::set_level(spdlog::level::trace);
        spdlog::flush_on(spdlog::level::err);
    }

    Logger::~Logger()
    {
        if (s_AsyncLogger)
        {
            s_AsyncLogger->flush();
            s_AsyncLogger.reset();
        }
        spdlog::shutdown();
    }

    void Logger::LogImpl(ELogLevel level, std::string_view target, std::string_view formatted)
    {
        if (!s_AsyncLogger)
        {
            return;
        }

        static TStaticArray<spdlog::level::level_enum, U32(ELogLevel::Max)> levelMap = {
            spdlog::level::trace,
            spdlog::level::debug,
            spdlog::level::info,
            spdlog::level::warn,
            spdlog::level::err,
        };

        s_AsyncLogger->log(levelMap[U32(level)], "[{}] {}", target, formatted);
    }

    void Logger::Flush()
    {
        s_AsyncLogger->flush();
    }

} // namespace worse