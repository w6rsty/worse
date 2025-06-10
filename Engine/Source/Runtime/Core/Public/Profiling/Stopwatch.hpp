#pragma once
#include <chrono>

namespace worse::profiling
{

    class Stopwatch
    {
    public:
        using TimePoint =
            std::chrono::time_point<std::chrono::high_resolution_clock>;

        Stopwatch()
        {
            reset();
        }

        void reset()
        {
            m_start = std::chrono::high_resolution_clock::now();
        }

        float elapsedMs() const
        {
            auto end = std::chrono::high_resolution_clock::now();
            return std::chrono::duration<float, std::milli>(end - m_start)
                .count();
        }

        float elapsedSec() const
        {
            auto end = std::chrono::high_resolution_clock::now();
            return std::chrono::duration<float>(end - m_start).count();
        }

    private:
        TimePoint m_start;
    };

} // namespace worse::profiling