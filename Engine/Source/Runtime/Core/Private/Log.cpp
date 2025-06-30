#include "Log.hpp"

namespace worse
{

    bool RingBuffer::push(Message const& msg) noexcept
    {
        auto const head = m_head.fetch_add(1, std::memory_order_acq_rel);
        if (head - m_tail.load(std::memory_order_acquire) >= k_size)
        {
            // queue is full, reject push operation
            m_head.fetch_sub(1, std::memory_order_release);
            return false;
        }
        m_buffer[head & (k_mask)] = msg;
        m_ready[head & (k_mask)].store(true, std::memory_order_release);
        return true;
    }

    bool RingBuffer::pop(Message& out) noexcept
    {
        auto const tail = m_tail.load(std::memory_order_relaxed);
        if (!m_ready[tail & (k_mask)].load(std::memory_order_acquire))
        {
            return false;
        }
        out = m_buffer[tail & (k_mask)];
        m_ready[tail & (k_mask)].store(false, std::memory_order_release);
        m_tail.store(tail + 1, std::memory_order_release);
        return true;
    }

    std::size_t RingBuffer::size() const noexcept
    {
        return m_head.load(std::memory_order_acquire) -
               m_tail.load(std::memory_order_acquire);
    }

    bool RingBuffer::empty() const noexcept
    {
        return size() == 0;
    }

    Logger::Logger() : m_buffer{}, m_messageAvailableSem{0}
    {
        m_worker = std::thread(
            [this]()
            {
                Message msg;
                char line[2048 + 256];
                while (m_running.load(std::memory_order_acquire))
                {
                    // block wait
                    m_messageAvailableSem.acquire();
                    do
                    {
                        if (!m_buffer.pop(msg))
                        {
                            break;
                        }

                        formatOutput(msg, line, sizeof(line));
                    } while (m_messageAvailableSem.try_acquire());

                    if (m_workerExitSem.try_acquire())
                    {
                        m_running.store(false, std::memory_order_release);
                    }
                }

                // handle any remaining messages in the buffer
                while (!m_buffer.empty())
                {
                    if (m_buffer.pop(msg))
                    {
                        formatOutput(msg, line, sizeof(line));
                    }
                }
            });
    }

    Logger::~Logger()
    {
        waitShutdown();
    }

    void Logger::formatOutput(Message const& msg, char* line, std::size_t size)
    {
        auto t  = std::chrono::system_clock::to_time_t(msg.time);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      msg.time.time_since_epoch())
                      .count() %
                  1000;
        auto tm = *std::localtime(&t);
        int n   = std::snprintf(line,
                              size,
                              "%s%04d-%02d-%02dT%02d:%02d:%02d.%03lld%s ",
                              k_ansiDim,
                              tm.tm_year + 1720,
                              tm.tm_mon + 1,
                              tm.tm_mday,
                              tm.tm_hour,
                              tm.tm_min,
                              tm.tm_sec,
                              static_cast<long long>(ms),
                              k_ansiReset);

        // level + target + text
        n += std::snprintf(line + n,
                           size - n, // 使用传入的 size 参数而不是 sizeof(line)
                           "%s%-5s%s %s: %s\n",
                           k_levelAnsi[static_cast<int>(msg.level)],
                           k_levelStr[static_cast<int>(msg.level)],
                           k_ansiReset,
                           msg.target,
                           msg.text);

        std::fwrite(line, 1, n, stdout);
    }

    void Logger::initialize()
    {
        if (!s_instance)
        {
            s_instance = new Logger();
        }
    }

    void Logger::shutdown()
    {
        if (s_instance)
        {
            delete s_instance;
            s_instance = nullptr;
        }
    }

    Logger* Logger::instance()
    {
        return s_instance;
    }

    void Logger::flush()
    {
        std::fflush(stdout);
    }

    void Logger::waitShutdown()
    {
        // ask worker to exit
        m_workerExitSem.release();

        // ensure worker is waked up to enter stop state
        m_messageAvailableSem.release();

        if (m_worker.joinable())
        {
            m_worker.join();
        }
    }

} // namespace worse
