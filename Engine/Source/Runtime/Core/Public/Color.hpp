#pragma once

namespace worse
{
    // Very simple 32-bit RGBA color
    struct Color
    {
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        float a = 1.0f;

        static constexpr Color White() { return Color{1.0f, 1.0f, 1.0f, 1.0f}; }
        static constexpr Color Black() { return Color{0.0f, 0.0f, 0.0f, 1.0f}; }
    };

} // namespace worse