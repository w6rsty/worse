#pragma once
#include "Types.hpp"

#include <chrono>

namespace worse::profiling
{

    class Stopwatch
    {
    public:
        using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

        Stopwatch()
        {
            reset();
        }

        void reset()
        {
            m_start = std::chrono::high_resolution_clock::now();
        }

        f32 elapsedMs() const
        {
            auto end = std::chrono::high_resolution_clock::now();
            return std::chrono::duration<f32, std::milli>(end - m_start)
                .count();
        }

        f32 elapsedSec() const
        {
            auto end = std::chrono::high_resolution_clock::now();
            return std::chrono::duration<f32>(end - m_start).count();
        }

    private:
        TimePoint m_start;
    };

} // namespace worse::profiling