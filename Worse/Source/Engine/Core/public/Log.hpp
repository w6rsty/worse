#pragma once
#include "base_type.hpp"

#include <atomic>
#include <array>
#include <chrono>
#include <cstring>
#include <format>
#include <thread>
#include <semaphore>

namespace Worse
{
    enum class Level : UByte
    {
        Trace,
        Debug,
        Info,
        Warn,
        Error,
        Fatal
    };
    constexpr std::array<char const*, 6> k_levelStr{
        "TRACE", "DEBUG", "INFO ", "WARN ", "ERROR", "FATAL"};
    constexpr std::array<char const*, 6> k_levelAnsi{
        "\x1b[90m",   // Trace – bright black
        "\x1b[36m",   // Debug – cyan
        "\x1b[32m",   // Info  – green
        "\x1b[33m",   // Warn  – yellow
        "\x1b[31m",   // Error – red
        "\x1b[41;97m" // Fatal – white on red
    };
    constexpr char const* k_ansiDim   = "\x1b[2m";
    constexpr char const* k_ansiReset = "\x1b[0m";

#ifndef WS_LOG_ACTIVE_LEVEL
#define WS_LOG_ACTIVE_LEVEL ::Worse::Level::Trace // compile time filter
#endif

    struct Message
    {
        std::chrono::system_clock::time_point time;
        Level level;
        Char target[32]; // module / subsystem name
        Char text[2048]; // formatted message
        std::thread::id tid;
    };

    // Fixed‑size MPSC ring‑buffer
    class RingBuffer
    {
    public:
        static constexpr Size k_size = 1024;
        static constexpr Size k_mask = k_size - 1;

        Bool push(Message const& msg) noexcept;
        Bool pop(Message& out) noexcept;
        [[nodiscard]] Size size() const noexcept;
        [[nodiscard]] Bool empty() const noexcept;

    private:
        alignas(64) std::array<Message, k_size> m_buffer{};
        alignas(64) std::array<std::atomic<Bool>, k_size> m_ready{};
        std::atomic<ULong> m_head{0};
        std::atomic<ULong> m_tail{0};
    };

    class Logger
    {
        Logger();
        ~Logger();
        static void formatOutput(Message const& msg, char* line, Size size);

    public:
        static void initialize();
        static void shutdown();
        static Logger* instance();

        static void flush();
        // Reject new message, wait for all messages to be processed, and exit
        void waitShutdown();

        template <Level L, class... Args>
        void log(char const* target, std::format_string<Args...> fmt,
                 Args&&... args)
        {
            if (!m_running.load(std::memory_order_acquire))
            {
                return;
            }

            if constexpr (static_cast<int>(L) <
                          static_cast<int>(WS_LOG_ACTIVE_LEVEL))
            {
                return;
            }
            Message msg;
            msg.time         = std::chrono::system_clock::now();
            msg.level        = L;
            Size target_len = std::min(strlen(target), sizeof(msg.target) - 1);
            memcpy(msg.target, target, target_len);
            msg.target[target_len] = '\0';
            auto res               = std::format_to_n(
                msg.text,
                sizeof(msg.text) - 4, // for ...\n
                fmt,
                std::forward<Args>(args)...);

            if (res.size >= sizeof(msg.text) - 4)
            {
                memcpy(msg.text + sizeof(msg.text) - 4, "...", 3);
                msg.text[sizeof(msg.text) - 1] = '\0';
            }
            else
            {
                msg.text[res.size] = '\0';
            }
            msg.tid = std::this_thread::get_id();

            if (m_buffer.push(msg)) // log may be dropped if the buffer is full
            {
                m_messageAvailableSem.release();
            }
        }

    private:
        static inline Logger* s_instance{nullptr};

        RingBuffer m_buffer;
        std::counting_semaphore<RingBuffer::k_mask> m_messageAvailableSem{0};
        std::binary_semaphore m_workerExitSem{0};
        std::thread m_worker;
        std::atomic<Bool> m_running{true};
    };

} // namespace Worse

// clang-format off
#define WS_LOG_TRACE(target, fmt, ...) do { auto* _logger = ::Worse::Logger::instance(); if (_logger) _logger->log<::Worse::Level::Trace>(target, fmt __VA_OPT__(,) __VA_ARGS__); } while(0)
#define WS_LOG_DEBUG(target, fmt, ...) do { auto* _logger = ::Worse::Logger::instance(); if (_logger) _logger->log<::Worse::Level::Debug>(target, fmt __VA_OPT__(,) __VA_ARGS__); } while(0)
#define WS_LOG_INFO(target,  fmt, ...) do { auto* _logger = ::Worse::Logger::instance(); if (_logger) _logger->log<::Worse::Level::Info >(target, fmt __VA_OPT__(,) __VA_ARGS__); } while(0)
#define WS_LOG_WARN(target,  fmt, ...) do { auto* _logger = ::Worse::Logger::instance(); if (_logger) _logger->log<::Worse::Level::Warn >(target, fmt __VA_OPT__(,) __VA_ARGS__); } while(0)
#define WS_LOG_ERROR(target, fmt, ...) do { auto* _logger = ::Worse::Logger::instance(); if (_logger) _logger->log<::Worse::Level::Error>(target, fmt __VA_OPT__(,) __VA_ARGS__); } while(0)
#define WS_LOG_FATAL(target, fmt, ...) do { auto* _logger = ::Worse::Logger::instance(); if (_logger) _logger->log<::Worse::Level::Fatal>(target, fmt __VA_OPT__(,) __VA_ARGS__); } while(0)
// clang-format on
