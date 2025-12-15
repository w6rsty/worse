#pragma once
#include "BaseTypes.hpp"

namespace worse
{
    // Very simple 32-bit RGBA color
    struct Color
    {
        F32 r = 0.0f;
        F32 g = 0.0f;
        F32 b = 0.0f;
        F32 a = 1.0f;

        static constexpr Color White()
        {
            return Color{1.0f, 1.0f, 1.0f, 1.0f};
        }
        static constexpr Color Black()
        {
            return Color{0.0f, 0.0f, 0.0f, 1.0f};
        }
    };

} // namespace worse