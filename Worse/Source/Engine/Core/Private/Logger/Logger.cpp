#include "Logger/Logger.hpp"
#include "Macro/Common.hpp"

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include <array>

namespace worse
{

    Logger& Logger::Instance()
    {
        static Logger instance;
        return instance;
    }

    void Logger::Initialize(Size queueSize /*= MaxQueueSize*/, Size maxFileSize /*= MaxLogFileSize*/)
    {
        spdlog::init_thread_pool(queueSize, 1);

        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto fileSink    = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("Worse.log", maxFileSize, 3);

        consoleSink->set_pattern("\x1b[90m%Y-%m-%d %T.%e\x1b[0m %^%-7l %v%$");
        fileSink->set_pattern("%Y-%m-%d %T.%e %-7l %v");

        auto logger = std::make_shared<spdlog::async_logger>(
            "WORSE",
            spdlog::sinks_init_list{consoleSink, fileSink},
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::discard_new);

        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);

        spdlog::set_level(spdlog::level::trace);
        spdlog::flush_on(spdlog::level::err);
    }

    void Logger::LogImpl(LogLevel level, std::string_view target, std::string_view formatted)
    {
        auto logger = spdlog::get("WORSE");
        if (!logger)
        {
            return;
        }

        constexpr std::array<spdlog::level::level_enum, s_cast<Size>(LogLevel::Max)> levelMap = {
            spdlog::level::trace,
            spdlog::level::debug,
            spdlog::level::info,
            spdlog::level::warn,
            spdlog::level::err,
        };

        logger->log(levelMap[s_cast<Size>(level)], "[{}] {}", target, formatted);
    }

} // namespace worse