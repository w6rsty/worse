#pragma once
#include <algorithm>
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

        [[nodiscard]] constexpr Vector2 operator*(float scalar) const noexcept
        {
            return Vector2{x * scalar, y * scalar};
        }

        [[nodiscard]] constexpr Vector2 operator/(float scalar) const noexcept
        {
            return Vector2{x / scalar, y / scalar};
        }

        constexpr Vector2 operator+=(Vector2 const& other) noexcept
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        constexpr Vector2 operator-=(Vector2 const& other) noexcept
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        constexpr Vector2 operator*=(float scalar) noexcept
        {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        constexpr Vector2 operator/=(float scalar) noexcept
        {
            x /= scalar;
            y /= scalar;
            return *this;
        }

        [[nodiscard]] constexpr float lengthSquare() const noexcept
        {
            return x * x + y * y;
        }

        [[nodiscard]] constexpr float length() const noexcept
        {
            return std::sqrt(lengthSquare());
        }

        [[nodiscard]] Vector2 clamp(float min, float max) const noexcept
        {
            return Vector2{std::clamp(x, min, max), std::clamp(y, min, max)};
        }

        float x;
        float y;
    };

} // namespace worse::math