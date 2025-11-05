#pragma once
#include "base_type.hpp"

#include <chrono>

namespace Worse::profiling
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

        Float elapsedMs() const
        {
            auto end = std::chrono::high_resolution_clock::now();
            return std::chrono::duration<Float, std::milli>(end - m_start).count();
        }

        Float elapsedSec() const
        {
            auto end = std::chrono::high_resolution_clock::now();
            return std::chrono::duration<Float>(end - m_start).count();
        }

    private:
        TimePoint m_start;
    };

} // namespace Worse::profiling