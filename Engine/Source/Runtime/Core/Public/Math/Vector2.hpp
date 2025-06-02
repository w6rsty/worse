#pragma once
#include <cmath>

namespace worse::math
{

    class Vector2
    {
    public:
        [[nodiscard]] constexpr Vector2() noexcept : x(0.0f), y(0.0f)
        {
        }

        [[nodiscard]] constexpr Vector2(float x, float y) noexcept : x(x), y(y)
        {
        }

        [[nodiscard]] constexpr Vector2
        operator+(Vector2 const& other) const noexcept
        {
            return Vector2{x + other.x, y + other.y};
        }

        [[nodiscard]] constexpr Vector2
        operator-(Vector2 const& other) const noexcept
        {
            return Vector2{x - other.x, y - other.y};
        }

        [[nodiscard]] float lengthSquare() const noexcept
        {
            return x * x + y * y;
        }

        [[nodiscard]] float length() const noexcept
        {
            return std::sqrt(lengthSquare());
        }

        float x;
        float y;
    };

} // namespace worse::math