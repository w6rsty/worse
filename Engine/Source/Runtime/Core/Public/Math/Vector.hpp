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

    class Vector3
    {
    public:
        [[nodiscard]] constexpr Vector3() noexcept : x(0.0f), y(0.0f), z(0.0f)
        {
        }

        [[nodiscard]] constexpr Vector3(float x, float y, float z) noexcept
            : x(x), y(y), z(z)
        {
        }

        [[nodiscard]] constexpr Vector3
        operator+(Vector3 const& other) const noexcept
        {
            return Vector3{x + other.x, y + other.y, z + other.z};
        }

        [[nodiscard]] constexpr Vector3
        operator-(Vector3 const& other) const noexcept
        {
            return Vector3{x - other.x, y - other.y, z - other.z};
        }

        [[nodiscard]] constexpr Vector3 operator*(float scalar) const noexcept
        {
            return Vector3{x * scalar, y * scalar, z * scalar};
        }

        [[nodiscard]] constexpr Vector3 operator/(float scalar) const noexcept
        {
            return Vector3{x / scalar, y / scalar, z / scalar};
        }

        constexpr Vector3 operator+=(Vector3 const& other) noexcept
        {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        constexpr Vector3 operator-=(Vector3 const& other) noexcept
        {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }

        constexpr Vector3 operator*=(float scalar) noexcept
        {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            return *this;
        }

        constexpr Vector3 operator/=(float scalar) noexcept
        {
            x /= scalar;
            y /= scalar;
            z /= scalar;
            return *this;
        }

        [[nodiscard]] constexpr float lengthSquare() const noexcept
        {
            return x * x + y * y + z * z;
        }

        [[nodiscard]] constexpr float length() const noexcept
        {
            return std::sqrt(lengthSquare());
        }

        float x;
        float y;
        float z;
    };

    class Vector4
    {
    public:
        [[nodiscard]] constexpr Vector4() noexcept
            : x(0.0f), y(0.0f), z(0.0f), w(0.0f)
        {
        }

        [[nodiscard]] constexpr Vector4(float x, float y, float z,
                                        float w) noexcept
            : x(x), y(y), z(z), w(w)
        {
        }

        [[nodiscard]] constexpr Vector4
        operator+(Vector4 const& other) const noexcept
        {
            return Vector4{x + other.x, y + other.y, z + other.z, w + other.w};
        }

        [[nodiscard]] constexpr Vector4
        operator-(Vector4 const& other) const noexcept
        {
            return Vector4{x - other.x, y - other.y, z - other.z, w - other.w};
        }

        [[nodiscard]] constexpr Vector4 operator*(float scalar) const noexcept
        {
            return Vector4{x * scalar, y * scalar, z * scalar, w * scalar};
        }

        [[nodiscard]] constexpr Vector4 operator/(float scalar) const noexcept
        {
            return Vector4{x / scalar, y / scalar, z / scalar, w / scalar};
        }

        constexpr Vector4 operator+=(Vector4 const& other) noexcept
        {
            x += other.x;
            y += other.y;
            z += other.z;
            w += other.w;
            return *this;
        }

        constexpr Vector4 operator-=(Vector4 const& other) noexcept
        {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            w -= other.w;
            return *this;
        }

        constexpr Vector4 operator*=(float scalar) noexcept
        {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            w *= scalar;
            return *this;
        }

        constexpr Vector4 operator/=(float scalar) noexcept
        {
            x /= scalar;
            y /= scalar;
            z /= scalar;
            w /= scalar;
            return *this;
        }

        float x;
        float y;
        float z;
        float w;
    };

} // namespace worse::math