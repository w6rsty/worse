#pragma once
#include "base_type.hpp"

namespace Worse
{
    // Very simple 32-bit RGBA color
    struct Color
    {
        Float r = 0.0f;
        Float g = 0.0f;
        Float b = 0.0f;
        Float a = 1.0f;

        static constexpr Color White()
        {
            return Color{1.0f, 1.0f, 1.0f, 1.0f};
        }
        static constexpr Color Black()
        {
            return Color{0.0f, 0.0f, 0.0f, 1.0f};
        }
    };

} // namespace Worse