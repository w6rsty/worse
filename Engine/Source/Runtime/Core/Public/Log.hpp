#pragma once

#include <atomic>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <format>
#include <thread>
#include <semaphore>

namespace worse
{

    enum class Level : std::uint8_t
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
#define WS_LOG_ACTIVE_LEVEL ::worse::Level::Trace // compile time filter
#endif

    struct Message
    {
        std::chrono::system_clock::time_point time;
        Level level;
        char target[32]; // module / subsystem name
        char text[2048]; // formatted message - 增加大小以适应长验证层消息
        std::thread::id tid;
    };

    // Fixed‑size MPSC ring‑buffer
    class RingBuffer
    {
    public:
        static constexpr std::size_t k_size = 1024;
        static constexpr std::size_t k_mask = k_size - 1;

        bool push(Message const& msg) noexcept;
        bool pop(Message& out) noexcept;
        [[nodiscard]] std::size_t size() const noexcept;
        [[nodiscard]] bool empty() const noexcept;

    private:
        alignas(64) std::array<Message, k_size> m_buffer{};
        alignas(64) std::array<std::atomic<bool>, k_size> m_ready{};
        std::atomic<uint64_t> m_head{0};
        std::atomic<uint64_t> m_tail{0};
    };

    class Logger
    {
        Logger();
        ~Logger();
        static void formatOutput(Message const& msg, char* line,
                                 std::size_t size);

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
            if (!m_running)
            {
                return;
            }

            if constexpr (static_cast<int>(L) <
                          static_cast<int>(WS_LOG_ACTIVE_LEVEL))
            {
                return;
            }
            Message msg;
            msg.time  = std::chrono::system_clock::now();
            msg.level = L;
            std::strncpy(msg.target, target, sizeof(msg.target) - 1);
            msg.target[sizeof(msg.target) - 1] = '\0';
            auto res                           = std::format_to_n(
                msg.text,
                sizeof(msg.text) - 4, // 保留4个字符以便在需要时添加省略号
                fmt,
                std::forward<Args>(args)...);

            if (res.size >= sizeof(msg.text) - 4)
            {
                std::strncpy(msg.text + sizeof(msg.text) - 4, "...", 4);
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
        std::atomic<bool> m_running{true};
    };

} // namespace worse

// clang-format off
#define WS_LOG_TRACE(target, fmt, ...) ::worse::Logger::instance()->log<::worse::Level::Trace>(target, fmt __VA_OPT__(,) __VA_ARGS__)
#define WS_LOG_DEBUG(target, fmt, ...) ::worse::Logger::instance()->log<::worse::Level::Debug>(target, fmt __VA_OPT__(,) __VA_ARGS__)
#define WS_LOG_INFO(target,  fmt, ...) ::worse::Logger::instance()->log<::worse::Level::Info >(target, fmt __VA_OPT__(,) __VA_ARGS__)
#define WS_LOG_WARN(target,  fmt, ...) ::worse::Logger::instance()->log<::worse::Level::Warn >(target, fmt __VA_OPT__(,) __VA_ARGS__)
#define WS_LOG_ERROR(target, fmt, ...) ::worse::Logger::instance()->log<::worse::Level::Error>(target, fmt __VA_OPT__(,) __VA_ARGS__)
#define WS_LOG_FATAL(target, fmt, ...) ::worse::Logger::instance()->log<::worse::Level::Fatal>(target, fmt __VA_OPT__(,) __VA_ARGS__)
// clang-format on
